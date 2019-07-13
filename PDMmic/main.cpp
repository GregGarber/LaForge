/** TODO rewrite CIC to use non floating point since there appear to be no floats used. May need long or int64_t though.
    TODO better pre-warming for CIC
    TODO Do a better job of optimizing Wav's endiness test. Should really test at compile time and conditionally
	compile correct support.
    TODO may need sub-byte resolution so R can be other than 8 or 16
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <wiringPi.h>
#include "LUTs.h"
#include "CIC.h"
#include "Wav.h"

//pi@pi3:~/Sisonic9Jun19 $ g++ -std=c++14 myspi.cpp -o myspi -lpthread

//#define BUFFY_SIZE 1024
//#define BUFFY_SIZE 65534
//#define BUFFY_SIZE 8192
//#define BUFFY_SIZE 4096
#define BUFFY_SIZE 16384
#define BUFFY_CNT 4
#define SPIDEV "/dev/spidev0.0"
#define SEL 15


//static uint16_t rxbuff[BUFFY_CNT][BUFFY_SIZE];//={0,0};
static uint8_t rxbuff[BUFFY_CNT][BUFFY_SIZE];//={0,0};
//static double buff[BUFFY_CNT][BUFFY_SIZE*8];//={0,0};
enum BufStatus {
	Empty=1,
	ReadOnly,
	WriteOnly
} billboard[BUFFY_CNT];

bool carryOn=true;
int writeIndex=0;
int readIndex=0;
std::thread spiReaderThread;
std::thread fileWriterThread;

struct spi_ioc_transfer xfer[BUFFY_CNT][1]; 

int fd; //file handle for spi dev
int outfh;//output file handle
int rdmode = 0;
int wrmode = 1;//not sure

// Mic Datasheet says max clock rate is 4.8MHz
// RPI SPI has to be 250MHz/(2*N) where N is an integer
// Best speed should be [I] >> 250/(2*27) =  4.6296
// With a decimation of 32, that's 144,675 PCM samples per second
// Which isn't standard, but audacity doesn't seem to mind weird rates
//uint32_t rdspeed = 3072000;
//uint32_t rdspeed = 1024000;
   //uint32_t rdspeed = 3906250;
//   uint32_t rdspeed = 3125000;
   uint32_t rdspeed = 4096000;//a lot of noise, but actual signal too
//   uint32_t rdspeed = 5000000;//may be nothing but noise
uint32_t wrspeed = rdspeed;

	int opt, ret, dataCount;
	int finished = 0;
	unsigned int pdmSamplingF=rdspeed, pcmSamplingF, pdmBufLen=BUFFY_SIZE, pcmBufLen;
//	uint8_t* pdmBuf;
	int16_t* pcmBuf;
Wave mySound;
float frameData[1];
CIC *cic ;

//reads SPI into buffer

void spiReader() {
	int                     status;
	fd=open(SPIDEV,O_RDWR | O_SYNC);
	if(fd<0) {
		perror("Failed to open SPI DEV File");
		exit(1);
	}
	if(ioctl(fd, SPI_IOC_RD_MODE, &rdmode)<0){
		perror("Failed to set SPI RD mode");
	}
	if(ioctl(fd, SPI_IOC_WR_MODE, &wrmode)<0){
		perror("Failed to set SPI WR mode");
	}

	if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &rdspeed)<0){
		perror("Failed to set SPI RD speed");
	}
	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &wrspeed)<0){
		perror("Failed to set SPI WR speed");
	}

	//put mic into nomal mode in case it was asleep
	//also a good time to flush gunk when it starts
	xfer[0][0].speed_hz=1024000;
	//can take up to 15ms to wake from sleep
	//if more than 10ms passes without clock signal, would fall back to sleep
	for(int i=0;i<16;i++){
		status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer[0]);
		usleep(1000);
	}
	//now it should be able to switch directly to any other mode
	xfer[0][0].speed_hz=rdspeed;

	for(int i=0;i<BUFFY_CNT;i++){
		memset(xfer[i], 0, sizeof xfer[i]);
		memset(rxbuff[i],0,BUFFY_SIZE);
		xfer[i][0].rx_buf = (unsigned long) rxbuff[i];
		xfer[i][0].len = BUFFY_SIZE;
	}
	//in case more gunk to flush which shouldn't be in file
	for(int i=0;i<16;i++){
		status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer[0]);
	}

	while(carryOn) {
		if(billboard[writeIndex]==BufStatus::WriteOnly){
			//here
			status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer[writeIndex]);
			if (status < 0) {
				perror("SPI_IOC_MESSAGE");
				return;
			}
			billboard[writeIndex]=BufStatus::ReadOnly;
			writeIndex=(writeIndex>=(BUFFY_CNT-1)) ? 0:writeIndex+1;
		}else{
			printf("Feed the starving thread please!\n");
		}
	}
	//put mic back into low power mode
	xfer[0][0].speed_hz=102400;
	xfer[0][0].len=8;
	status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer[0]);
}


//writes buffer to disk
void fileWriter() {
/* 
double lilbuf[16];
size_t blen=sizeof(double)*8;
*/
	
	while(carryOn) {
		if(billboard[readIndex]==BufStatus::ReadOnly){
for(int i=0; i< BUFFY_SIZE; i++){
	//rxbuff[readIndex][i] = cic->filter( d2bLUT[rxbuff[readIndex][i]], 8 ) ;

/*
potentially useful for non-ultrasonic mode.
	memcpy(&lilbuf[0],&d2bLUT[rxbuff[readIndex][i] ], blen);
	memcpy(&lilbuf[8],&d2bLUT[rxbuff[readIndex][i+1]], blen);
	frameData[0]= cic->filter( lilbuf, 16 ) ;
*/
	frameData[0]=  cic->filter( d2bLUT[rxbuff[readIndex][i]], 8 );

        waveAddSample(&mySound, frameData);
}
//			write(outfh,rxbuff[readIndex], xfer[readIndex][0].len);
			billboard[readIndex]=BufStatus::WriteOnly;
			readIndex=(readIndex>=(BUFFY_CNT-1)) ? 0:readIndex+1;
		}
	}
}

void dumpBillboard(){
	for(int i=0;i<BUFFY_CNT;i++){
		printf("idx:%d\tval:%d\n",i,billboard[i]);
	}
}

/*
void tcpStream(){
	    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sock
}
*/
int main () {

  //mySound = makeWave(384000,1,16);//384000KSamp/sec, mono 16-bit
  mySound = makeWave(512000,1,8);
  //mySound = makeWave(512000,1,16);
  //mySound = makeWave(256000,1,16);
  waveSetDuration( &mySound, 11 );//seconds

  	//cic = new CIC(16, 2, 8); // 1 black stripe. 16x decimation likely can't work for bats
  	cic = new CIC(8, 2, 6); // 1 black stripe. 16x decimation likely can't work for bats
	cic->setAttenuation(6.0); 
for(int i=0;i<1000;i++){
	cic->filter(d2bLUT[i&0xff],8);
}
cic->dump();
	carryOn=true;
	//outfh=open("/tmp/first.dat",O_WRONLY|O_TRUNC|O_CREAT|S_IROTH|S_IROTH);
	//outfh=open("/tmp/first.dat",O_RDWR|O_TRUNC|O_CREAT|S_IROTH|S_IROTH);
	//outfh=open("/dev/shm/first.dat",O_RDWR|O_TRUNC|O_CREAT|S_IROTH|S_IROTH);
	outfh=open("ramdrive/first.dat",O_RDWR|O_TRUNC|O_CREAT);
	if(outfh<0){
		perror("opening output file first.dat");
		exit(0);
	}

	wiringPiSetupPhys(); //WILL USE Physical pin header numbers
	pinMode(SEL, INPUT);
	pullUpDnControl(SEL,PUD_DOWN);
	//digitalWrite(SEL, 0);

	for(int i=0;i<BUFFY_CNT;i++){
		billboard[i]=BufStatus::WriteOnly;
	}



/*
	spiReaderThread = std::thread(spiReader,0);
	fileWriterThread = std::thread(fileWriter,1);
*/
	spiReaderThread = std::thread(spiReader);
	fileWriterThread = std::thread(fileWriter);

	cpu_set_t cpuset;
	bool failed=0;

     // struct sched_param is used to store the scheduling priority
     struct sched_param params;
 
     // We'll set the priority to the maximum.
     params.sched_priority = sched_get_priority_max(SCHED_FIFO);

	CPU_ZERO(&cpuset);
	CPU_SET(2, &cpuset);
	pthread_setaffinity_np(fileWriterThread.native_handle(), sizeof(cpu_set_t), &cpuset);
	std::cout << "Trying to set thread realtime prio = " << params.sched_priority << std::endl;
 
     // Attempt to set thread real-time priority to the SCHED_FIFO policy
     if ( pthread_setschedparam(fileWriterThread.native_handle(), SCHED_FIFO, &params) != 0) {
         // Print the error
	perror("perr says");
         std::cout << "Unsuccessful in setting Writer thread realtime prio" << std::endl;
	failed |=1;
     }

	CPU_ZERO(&cpuset);
	CPU_SET(3, &cpuset);
	pthread_setaffinity_np(spiReaderThread.native_handle(), sizeof(cpu_set_t), &cpuset);

	std::cout << "Trying to set thread realtime prio = " << params.sched_priority << std::endl;
 
     // Attempt to set thread real-time priority to the SCHED_FIFO policy
     if ( pthread_setschedparam(spiReaderThread.native_handle(), SCHED_FIFO, &params) != 0) {
	perror("perr says");
         // Print the error
         std::cout << "Unsuccessful in setting Reader thread realtime prio" << std::endl;
	failed |=1;
     }


     if(failed){
	     carryOn = false;
     }else{
	     sleep(10);
	     carryOn = false;
     }

	spiReaderThread.join();
	std::cout << "Spi Thread re-joined"<<std::endl;
	fileWriterThread.join();
	std::cout << "Writer Thread re-joined"<<std::endl;

	dumpBillboard();

    waveToFile( &mySound, "ramdrive/froggy.wav");
    waveDestroy( &mySound );
cic->dump();

	return 0;
}

