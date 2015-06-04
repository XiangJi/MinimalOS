void main2();
void handleTimerInterrupt(int segment, int sp);
void killProcess(int x);
void terminate();
void executeProgram(char* name);
void writeFile(char* filename,char inbuf[]);
void readFile(char* filename, char outbuf[]);
void deleteFile(char* filename);
void directory();
void writeSector(char* buffer, int sector);
void eraseSector(int sector);
void readSector(char* buffer, int sector);
void printString(char* str);
void readString(char* buffer);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void changeDirectory(char* path);
void printWorkingDirectory();
int MOD(int x, int y);
int DIV(int x, int y);


int CurrentProcess;
int ProcessTableActive [8];
int ProcessTableSP[8];

int Sector;
char CurrentPath[8];


void main()
{
main2();
}

void main2()

{
int i;
for(i=0;i<8;i++)
{
	ProcessTableActive[i]=0;
	ProcessTableSP[i]=0xff00;
}
  CurrentProcess = 0;

  for(i=0; i<8; i++)
      CurrentPath[i] = 0;
      CurrentPath[0] = '/';
  
  //Set the root dir sector
  Sector = 2;

  makeTimerInterrupt();
  makeInterrupt21();
  interrupt(0x21, 9, "shell", 0, 0);
  while(1);
}


void handleTimerInterrupt(int segment, int sp){
	int i;

	setKernelDataSegment();
	
	//First time from kernel to shell
	if(segment==0x1000 && ProcessTableActive[CurrentProcess]==1)
	{
		sp=ProcessTableSP[CurrentProcess];
		segment=(CurrentProcess+2)*0x1000;
	}

	//RR after leave kernel
	if(segment!=0x1000)
	{
		ProcessTableSP[CurrentProcess]=sp;
		for(i=CurrentProcess+1;i<CurrentProcess+9;i++)
		{
			if(ProcessTableActive[MOD(i,8)]==1)	
			{
				CurrentProcess=MOD(i,8);
				segment =(CurrentProcess+2)*0x1000;
				sp = ProcessTableSP[CurrentProcess];
				break;
			}
		}
	}

if (segment == 0x1000) {
putInMemory(0xB000, 0x8162, 'K');
putInMemory(0xB000, 0x8163, 0x7);
}
else if (segment == 0x2000) {
putInMemory(0xB000, 0x8164, '0');
putInMemory(0xB000, 0x8165, 0x7);
}
else if (segment == 0x3000) {
putInMemory(0xB000, 0x8166, '1');
putInMemory(0xB000, 0x8167, 0x7);
}
else {
putInMemory(0xB000, 0x8160, 'X');
putInMemory(0xB000, 0x8161, 0x7);
}
//	printString("Tic");
	restoreDataSegment();
	returnFromTimer(segment, sp);
}

void terminate(){
setKernelDataSegment();
ProcessTableActive[CurrentProcess] = 0;
ProcessTableSP[CurrentProcess] = 0xFF00;
restoreDataSegment();
while(1);
}

void killProcess(int x){
setKernelDataSegment();
ProcessTableActive[x] = 0;
ProcessTableSP[x] = 0xFF00;
restoreDataSegment();
}

void executeProgram(char* name){
char prog[4096];
	int i;
	int seg_num;
	setKernelDataSegment();
	// finde the free entry
	for(seg_num=0;seg_num<8;seg_num++)
	{

		if(ProcessTableActive[seg_num]==0)
		{
			break;
		}
		
	}

	restoreDataSegment();	

	readFile(name, prog);

	for(i=0; i<4096; i++) 
	{
        	putInMemory((seg_num+2)*0x1000, i, prog[i]);
    	}
	initializeProgram((seg_num+2)*0x1000);
	
	//update process table
	setKernelDataSegment();
	ProcessTableActive[seg_num]=1;
	ProcessTableSP[seg_num] = 0xff00;
	restoreDataSegment();	 
 /* char prog[4096];
   int i=0;
   int segment;
   readFile(name,prog);
   setKernelDataSegment();
   for(i=0; i<8; i++)
        {   if(ProcessTableActive[i] == 0)
            {  segment = 0x2000 + i*0x1000;
               ProcessTableActive[i] = 1;
               CurrentProcess = i;
               break; }
        }
   restoreDataSegment();
   while(i < 4096){
     if(i==0xA000)
        break;
     putInMemory(segment,0x0000+i,prog[i]);
     i++; 
   }
  initializeProgram(segment);*/
}

void writeFile(char* filename,char inbuf[]){
   int i=0,num_sec,j=0,k=0,count=0;
   char map[512];
   char dir[512];
   readSector(map,1);
   readSector(dir,2);
   
   while(map[i] != 0x0){
     i++;
   }
   //update map
   map[i] = 0xFF;
   writeSector(map,1);
  //update dir 
  while(dir[j*32] != 0x0){
    j++;
  }
  while(k<6){
     dir[j*32+k] = filename[k];
     k++;
  }
     dir[j*32+k] = i+1; 
 
  writeSector(dir,2);
 //write to sector;
  writeSector(inbuf,i+1);
}

void readFile(char* filename, char outbuf[]){
  int i,j,k=0,l,index,count;
  int file_sectors[26];
  char dir[512];
  char tempbuf[512];
  int sectorIndex;
  setKernelDataSegment();
  sectorIndex = Sector;
  restoreDataSegment();
  readSector(dir,sectorIndex);
  //Search for filename in dir[512]
  for(i=0; i<16; i++){
    count=0;
    for(j=i*32;j<(i*32)+6;j++){
       if(filename[count] == dir[j]){
         count++;
       }
       else 
         break;
    }
    if(count == 6){
        index = (i*32) + 6;         // i*32 
        break;
    }
  }
  //get sectors allocated to "filename" from dir[512]
  while(dir[index+k] != 0x0){
     file_sectors[k] = dir[index+k]; 
     //readSector(outbuf,file_sectors[k]);
     k++;
  }
  //read from each sector and concatenate
  j=0;
  while(j < k){
      readSector(tempbuf,file_sectors[j]);
      l=0;
      while(l<512){
         outbuf[(512*j)+l] = tempbuf[l];
         l++;
      }
      j++;
     // tempbuf[0]='\0';
  }   
   
}

void deleteFile(char* filename){
  char map[512],dir[512];
  int file_sectors[26];
  int i,j,k=0,l,m,index,count;
  static int empty[512] = {0};
  //read map and directory into map[512],dir[512] respectively.
  readSector(map,1);
  readSector(dir,2);

  //Search for filename in dir[512]
  for(i=0; i<16; i++){
    count=0;
    for(j=i*32;j<(i*32)+6;j++){
       if(filename[count] == dir[j]){
         count++;
       }
       else 
         break;
    }
    if(count == 6){
        index = (i*32) + 6;         // i*32 
        break;
    }
  }
  //get sectors allocated to "filename" from dir[512]
  while(dir[index+k] != 0x0){
     file_sectors[k] = dir[index+k]; 
     writeSector(empty,file_sectors[k]);
     k++;
  }
   
  //Erase sectors from dir and map
  
  for(l=0; l<k;l++){
     map[file_sectors[k]-1] = 0x0; //erase from map
     writeSector(empty,file_sectors[l]);
  }
  
  for(m=0; m<32;m++){
     dir[i*32+m] = 0x0;   //erase from dir
  }
  //write back map and dir
  writeSector(map,1);
  writeSector(dir,2);
  
}

void changeDirectory(char* path)
{
    int index;
    int i;
    int j;
    int sectorIndex;
    int point;
    char stp;
    char buffer[512];

    sectorIndex = 0;
    point = 0;
    stp = 0;

    if(path[0] == '/' && path[1] != 0) {
        readSector(buffer, 2);
    }
    else if(path[0] == '/' && path[1] == 0) {
        setKernelDataSegment();
        CurrentPath[0] = '/';
        restoreDataSegment();
        for(index=1; index<7; ++index)
        {
            setKernelDataSegment();
            CurrentPath[index] = 0;
            restoreDataSegment();
        }
        setKernelDataSegment();
        Sector = 2;
        restoreDataSegment();
        return;
    }
    else {
        return;
    }

    for(i=0; i<512; i+=32)
    {
        if(buffer[i] == path[1] && buffer[i+1] == path[2] && \
           buffer[i+2] == path[3] && buffer[i+3] == path[4] && \
           buffer[i+4] == path[5] && buffer[i+5] == path[6])
        {
        
            for(j=i+6; j<(i+31); j++)
            {
                if(buffer[j] != 0)
                {
                    sectorIndex = buffer[j];
                    point = buffer[i+31];
       
                    break;
                }
            }
        }
    }

    if(point == 0x02)
    {
        for(index=0; index<7; ++index)
        {
            stp = path[index];
            setKernelDataSegment();
            CurrentPath[index] = stp;
            restoreDataSegment();
        }

        setKernelDataSegment();
        Sector = sectorIndex;
        restoreDataSegment();
    }
    else
    {
        printString(path);
        setKernelDataSegment();
        printString("It is not a directory\n");
        restoreDataSegment();
    }
}

void printWorkingDirectory()
{
    setKernelDataSegment();
    printString(CurrentPath);
    restoreDataSegment();
}


void directory(){
   int i,j;
   int sectorNum;
   char buffer[512];
   setKernelDataSegment();
   sectorNum = Sector;
   restoreDataSegment();
   readSector(buffer,sectorNum);
  for(i=0;i<16;i++){  
    for(j=i*32;j<(i*32)+6;j++){
       if(buffer[i*32] == 0x0)
         break;
       interrupt(0x10,0xe*256+buffer[j],0,0,0);   
       if(j == (i*32)+5){
         interrupt(0x10,0xe*256+0x0A,0,0,0);   
         interrupt(0x10,0xe*256+0x0D,0,0,0);   
       }
    }
  } 
}

void handleInterrupt21(int ax, int bx, int cx, int dx){
 switch(ax){
   case 0:
      printString(bx);
      break;
   case 1:
      readString(bx);
      break;
   case 2:
      readSector(bx,cx);
      break;
   case 3:
      directory();
      break;
   case 4:
      deleteFile(bx);      
      break;
   case 5:
      terminate();      
      break;
   case 6:
      readFile(bx,cx);   
      break;
   case 8:
      writeFile(bx,cx);   
      break;
   case 9:
      executeProgram(bx);
      break;
   case 10:
      killProcess(bx);
      break;
   case 11:
      changeDirectory(bx);
      break;
   case 12:
      printWorkingDirectory();
      break;
   }
}


int DIV(int x, int y){
   int q = 0,p,quotient;
   while((p = y*q) < x){
     q=q+1;
   }
   if(p == x)
     quotient=q;
   else
     quotient=q-1;

   return quotient;
}

int MOD(int x, int y){
   while(x >= y)   
    x=x-y;
   
   return x; 
}
void writeSector(char* buf, int sector){
   int AX,CX,DX;
   AX = 3*256 + 1;
   CX = DIV(sector,36)*256 + MOD(sector,18) + 1;    
   DX = MOD(DIV(sector,18),2)*256; 
   interrupt(0x13,AX,buf,CX,DX);
}

void readSector(char* buffer, int sector){
   
   int AX,CX,DX;
   AX = 2*256 + 1;
   CX = DIV(sector,36)*256 + MOD(sector,18) + 1;    
   DX = MOD(DIV(sector,18),2)*256; 
   interrupt(0x13,AX,buffer,CX,DX);

}

void printString(char* str){
   int  i=0;
   while(str[i] != '\0'){
       interrupt(0x10,0xe*256+str[i],0,0,0);
       i=i+1;
   }
  
}

void readString(char* buffer){
  int i=0,k;
  while((k=interrupt(0x16,0,0,0,0)) != 0xd){
     if(k == 0x8 && i > 0){
        interrupt(0x10,0xe*256+k,0,0,0);
        i=i-1;
     }
     else { 
        buffer[i]=k;
        interrupt(0x10,0xe*256+buffer[i],0,0,0);
        i=i+1;
        }
      
  }
  buffer[i] = 0xA;
  buffer[i+1] = 0x0;
}

