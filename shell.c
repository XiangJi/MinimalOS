int strcmp(char* str1, char* str2);

void main()
{
    char command[50];
    char prog[10];
    char arg[20];
    char buffer[13312];
    int i,k,j,count;
    char line[100];
    char path[8];
//    enableInterrupts(); 
    while(1){
    enableInterrupts();
       i = k = 0;

       interrupt(0x21, 12, 0, 0, 0);
       interrupt(0x21, 0,"\r\nSHELL:", 0, 0);
       interrupt(0x21, 0, "$", 0, 0);
       interrupt(0x21, 1,command, 0, 0);
       interrupt(0x21, 0,"\r\n", 0, 0);
        
       if(strcmp("dir",command))
	   interrupt(0x21,3,0,0,0);
       else
         { 
            while((prog[i]=command[i]) != 0x20) 
	      i++;	
            while((arg[k]=command[k+i+1]) != '\0') 
	      k++;
	 }	

       if(strcmp("type",prog)){
	      interrupt(0x21,6,arg,buffer,0);
	      interrupt(0x21,0,buffer,0,0);
	  } 

       else if(strcmp("del",prog))   
	     interrupt(0x21,4,arg,0,0);
       else if(strcmp("create",prog)){
	      buffer[0]='\0';
	      count=0;
	      line[0]='\0'; 
	      do
		{
                   line[0]='\0';
		   j=0;
	           interrupt(0x21, 1, line, 0, 0);
		   while((buffer[count]=line[j]) != '\0'){
			count++;
			j++;
		      }
                }
               while(line[0] != 0x0A);
	       interrupt(0x21,8,arg,buffer,0);
	     }


	else if(strcmp("execute",prog)){
	      interrupt(0x21, 9, arg, 0x3000, 0);
              }
        else if(strcmp("kill",prog)){
              interrupt(0x21,10, arg, 0, 0);
	      }
	else if(strcmp("cd",prog)){
	interrupt(0x21,0,"\n",0, 0);
	if(arg[0] == '/'){
	for(i = 0; i < 7; ++i){
	if(arg[i] == 0xa || arg[i] == 0xd)
	    path[i] = 0;
	else
	    path[i] = arg[i];

}


}
	else{
	    path[0] = '/';
	    for(i = 0; i < 6; ++i){
                if(arg[i] == 0xa || arg[i] == 0xd)
		    path[1+i] = 0;
		else
		    path[1+i] = arg[i];

}
}
	interrupt(0x21,11,path,0,0);
	}
    }	

}
      
int strcmp(char* str1, char* str2){
   int i=0;
   while(str1[i] != '\0'){
      if(str1[i] != str2[i])
	 return 0;
      i++;  
   }
   return 1;
} 

