#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <time.h>

#include <unistd.h>

#include <sys/types.h>





// Function to calculate the CRC checksum for the payload

void calculate_crc(int message[568]) {

   int checksum[32];

   for(int i=0;i<32;i++){//initialise the array with zeros

        checksum[i]=0;

       }

   for(int i=8;i<519;i+=32)

   {

        for(int j=0;j<32;j++){ // Divide the payload into 16 32-bit blocks        

            checksum[j]^=message[i+j]; 

           // basically just taking bitwise xor of those 16 blocks(longitudinal parity check algorithm)

           }

   }

   for(int i=0;i<32;i++){

       message[i+520] =checksum[i]; //finally after doing that we just put that into the crc field in the message 

    }

}



// Function to introduce errors into the payload



void introduce_error(int message[568]) {

  int num_errors = rand() % 3;      //for no of errors to be between 0,1,2

  int i, error_position;

  for (i = 0; i < num_errors; i++) {

    error_position = (rand() % 512)+8;   //random position between 8 and 519

    message[error_position] ^= 1;       //flipping the bits

  }

}



int check(int message[568])

{

     int checksum1[32],checksum2[32];

     for(int i=0;i<32;i++){

         checksum1[i]=message[i+520];//get the checksum before transmission from crc field we calculated

      }

     for(int i=0;i<32;i++){ // do the same process we used to calculate checksum after the transmission is done

        checksum2[i]=0;

      }

     for(int i=8;i<519;i+=32)

     {

        for(int j=0;j<32;j++){        

            checksum2[j]^=message[i+j];

         } 

     }  

     for(int i=0;i<32;i++){//check if both the checksums are same or not

        if(checksum1[i]!=checksum2[i]){

           return 0;

         }

     }

     return 1;

}





int main() {

  srand(time(NULL));

  int message[568],buffer[568];

  int p[2],pid;

    //generating random message 

       for (int i=0; i<568; i++)

       {

        if (i>=8 && i<= 519){

            message[i] = rand() % 2;//for random binary bits in payload

            }

        else{

            message[i] = 0; //for this assignment the rest of the bits dont really matter

            }

       }

       calculate_crc(message);

  

  //taking error probability as input

  float ERROR_PROBABILITY;

  printf("Enter the probability of errors: \n");

  scanf("%f",&ERROR_PROBABILITY); 



  if(pipe(p) < 0){

       return 1;//error

  }

  pid= fork();

   if (pid < 0) { 

        fprintf(stderr, "Fork Failed");

        return 1;//fork failed since neither child nor parent process has been created

  }

  if(pid > 0){       //parent process

      close(p[0]); // close the reading end of the pipe first before writing

      while(1){

         for(int i=0;i<568;i++){

             buffer[i] = message[i];// we are basically keeping the message constant and send buffer to the read side 

         }

         if ((rand()%50)/50 < ERROR_PROBABILITY) {  // basically we are just making random probability < ERROR probability so higher the error probability more the chance of introducing an error

             introduce_error(buffer);

         }

         write(p[1],buffer,sizeof(buffer));//sending the buffer into the pipe

      }

     return 0;

  }

  if(pid == 0)     //child process

  {

     close(p[1]);//close the writing end of the pipe first before reading

      int retransmissions = 0;

      //basically in case of error we just retransmit the message again and again until sender and reciever recieve the same message

      while(1){

         read(p[0], buffer,sizeof(buffer));//recieving message(buffer) from the reading end

         retransmissions++;

         if(check(buffer)) //if message recieved is the message sent then stop the retransmissions and print the no of retransmissions

             break;

       }

     printf("No of retransmissions before success: %d \n" ,retransmissions);

     return 0;

  }

 

   

}
