/* Consumer with buffer inside
By : Ali Azarian
Date: 23th Oct. 2013
*/

//------- HEADERS -------
#include <stdio.h>
#include "xparameters.h"
#include "xtmrctr.h"
#include "xutil.h"
#include "xenv.h"
#include <memory_parameters.h>
#include "xio.h"

//------- DEFINES -------
#define TMRCTR_DEVICE_ID  XPAR_XPS_TIMER_0_DEVICE_ID
#define TIMER_COUNTER_0     0
#define L 256

//----- index tables -----
int index1 [BUFFER_SIZE];
int index2 [BUFFER_SIZE];
int index3 [BUFFER_SIZE];
//----- flag tables -----
char flag1[BUFFER_SIZE];
char flag2[BUFFER_SIZE];
char flag3[BUFFER_SIZE];
//----- data tables -----
int data1 [BUFFER_SIZE];
int data2 [BUFFER_SIZE];
int data3 [BUFFER_SIZE];

int fifo_index,consumer_index;
int consumer_data,fifo_data,fifo_data2,consumer_data2;
int consumer_hash_index,fifo_hash_index;


//--- consumer Variables ---

const int n=16;
int SIZE_floid = 256;
int ci,consumer_done=0;
int read_from_fifo=1;

int main (void)
{
    float u[n*n], v[n*n], u0[n*n], v0[n*n];
    float x, y, x0, y_0, f, r, U[2], V[2], s, t;
    int i, j, i0, j_0, i1, j_1,m;
    int k;
    float visc=1.25;
    float dt=2.36;
    int start,end;
    
    xil_printf("----------------------- START ------------------------ ! \n\r");
    int MASK_LENGTH= 10;//(int) ceil((log(BUFFER_SIZE))/log(2));
    int MASK = ((1 << MASK_LENGTH)-1);

    //-- Flag Initializations
    for (i=0; i<BUFFER_SIZE; i++)
    {
        flag1[i]=0;
        flag2[i]=0;
        flag3[i]=0;
    }
    

    XTmrCtr timer;
    XTmrCtr_Initialize(&timer, XPAR_XPS_TIMER_0_DEVICE_ID);

    XTmrCtr_Reset(&timer, TIMER_COUNTER_0);
    start = XTmrCtr_GetValue(&timer, TIMER_COUNTER_0);

    // Start timer
    XTmrCtr_Start(&timer, TIMER_COUNTER_0);

    //------- LOOP FOR IMAGES -------
    for (k=0; k < Number_of_Images; k++)  // loop for Images 
    {
	  consumer_index = ci; // for nested loop can be i*n+j

        while (consumer_index<SIZE_floid)
        {

            
            if (read_from_fifo ==1)
            {
                getfsl(fifo_index,0);
                if (fifo_index!=999999999) 
                {    						
                    getfsl(fifo_data,0);
                    getfsl(fifo_data2,0);
                    xil_printf("READ %d FROM FIFO \t",fifo_index);
                    
                    if (consumer_index == fifo_index && fifo_index!=999999999)
                    {   
                        // consume the data directly				
                        consumer_data = fifo_data;
                        consumer_data2 = fifo_data2;
                        xil_printf("»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» %d DIRECTLY CONSUMED !\n\r",consumer_index);
                        
                        consumer_index=consumer_index+1;//ci;
								consumer_done=1;
                        read_from_fifo=1;
                        //-- The rest of consumer's computing stage here   : output: consumer_index
                        u0[i+(n+2)*j] = consumer_data;  // u[i+n*j]
                        v0[i+(n+2)*j] = consumer_data2;  // v[i+n*j]
                        y = j<=n/2 ? j : j-n;
                        r = x*x+y*y;
                        if ( r==0.0 ) continue;
                        f = exp(-r*dt*visc);
                        U[0] = u0[i +(n+2)*j]; 
                        V[0] = v0[i +(n+2)*j];
                        U[1] = u0[i+1+(n+2)*j]; 
                        V[1] = v0[i+1+(n+2)*j];
                        u0[i +(n+2)*j] = f*( (1-x*x/r)*U[0] -x*y/r *V[0] );
                        u0[i+1+(n+2)*j] = f*( (1-x*x/r)*U[1] -x*y/r *V[1] );
                        v0[i+ (n+2)*j] = f*( -y*x/r *U[0] + (1-y*y/r)*V[0] );
                        v0[i+1+(n+2)*j] = f*( -y*x/r *U[1] + (1-y*y/r)*V[1] );
                        
                        
                    }

                }
						else  if (fifo_index==999999999)
							{
							xil_printf("==PRODUCER STOPPED AND consumer_index IS %d!\n\r",consumer_index);
							read_from_fifo=0; 
							//consumer_index++;
							}  
				}			
               
                    // compute the hash
                    consumer_hash_index = consumer_index & MASK;   // here index is not refreshing
                    fifo_hash_index = fifo_index & MASK;
                    
                    // Load from memory
                    if (flag1[consumer_hash_index]==1 && index1[consumer_hash_index]==consumer_index)
                    {
                        flag1[consumer_hash_index]=0;
                        consumer_data= data1[consumer_hash_index];

                        xil_printf("========== LOAD consumer_index %d from T1 ================!\n\r",consumer_index);
								consumer_index++;
								consumer_done=1;
                        //-- The rest of consumer's computing stage here : output: consumer_index
                        u0[i+(n+2)*j] = consumer_data;  // u[i+n*j]
                        v0[i+(n+2)*j] = consumer_data2;  // v[i+n*j]
                        y = j<=n/2 ? j : j-n;
                        r = x*x+y*y;
                        if ( r==0.0 ) continue;
                        f = exp(-r*dt*visc);
                        U[0] = u0[i +(n+2)*j]; 
                        V[0] = v0[i +(n+2)*j];
                        U[1] = u0[i+1+(n+2)*j]; 
                        V[1] = v0[i+1+(n+2)*j];
                        u0[i +(n+2)*j] = f*( (1-x*x/r)*U[0] -x*y/r *V[0] );
                        u0[i+1+(n+2)*j] = f*( (1-x*x/r)*U[1] -x*y/r *V[1] );
                        v0[i+ (n+2)*j] = f*( -y*x/r *U[0] + (1-y*y/r)*V[0] );
                        v0[i+1+(n+2)*j] = f*( -y*x/r *U[1] + (1-y*y/r)*V[1] );
								

								if (fifo_index!=999999999) 
									read_from_fifo=1;
							

                    }
                    else if (flag2[consumer_hash_index]==1 && index2[consumer_hash_index]==consumer_index)
                    {
                        flag2[consumer_hash_index]=0;
                        consumer_data= data2[consumer_hash_index];
                       consumer_index++;
                        read_from_fifo=1;
                        consumer_done=1;
                        xil_printf("=================== LOAD consumer_index %d from T2 ================!\n\r",consumer_index);
                        //-- The rest of consumer's computing stage here  : output: consumer_index
                        u0[i+(n+2)*j] = consumer_data;  // u[i+n*j]
                        v0[i+(n+2)*j] = consumer_data2;  // v[i+n*j]
                        y = j<=n/2 ? j : j-n;
                        r = x*x+y*y;
                        if ( r==0.0 ) continue;
                        f = exp(-r*dt*visc);
                        U[0] = u0[i +(n+2)*j]; 
                        V[0] = v0[i +(n+2)*j];
                        U[1] = u0[i+1+(n+2)*j]; 
                        V[1] = v0[i+1+(n+2)*j];
                        u0[i +(n+2)*j] = f*( (1-x*x/r)*U[0] -x*y/r *V[0] );
                        u0[i+1+(n+2)*j] = f*( (1-x*x/r)*U[1] -x*y/r *V[1] );
                        v0[i+ (n+2)*j] = f*( -y*x/r *U[0] + (1-y*y/r)*V[0] );
                        v0[i+1+(n+2)*j] = f*( -y*x/r *U[1] + (1-y*y/r)*V[1] );

                    }
                    else if (flag3[consumer_hash_index]==1 && index3[consumer_hash_index]==consumer_index)
                    {
                        flag3[consumer_hash_index]=0;
                        consumer_data= data3[consumer_hash_index];
                        consumer_index++;
                        read_from_fifo=1;
                        consumer_done=1;
								consumer_done=1;
                        xil_printf("=================== LOAD consumer_index %d from T3 ================!\n\r",consumer_index);
                        //-- The rest of consumer's computing stage here : output: consumer_index
                        u0[i+(n+2)*j] = consumer_data;  // u[i+n*j]
                        v0[i+(n+2)*j] = consumer_data2;  // v[i+n*j]
                        y = j<=n/2 ? j : j-n;
                        r = x*x+y*y;
                        if ( r==0.0 ) continue;
                        f = exp(-r*dt*visc);
                        U[0] = u0[i +(n+2)*j]; 
                        V[0] = v0[i +(n+2)*j];
                        U[1] = u0[i+1+(n+2)*j]; 
                        V[1] = v0[i+1+(n+2)*j];
                        u0[i +(n+2)*j] = f*( (1-x*x/r)*U[0] -x*y/r *V[0] );
                        u0[i+1+(n+2)*j] = f*( (1-x*x/r)*U[1] -x*y/r *V[1] );
                        v0[i+ (n+2)*j] = f*( -y*x/r *U[0] + (1-y*y/r)*V[0] );
                        v0[i+1+(n+2)*j] = f*( -y*x/r *U[1] + (1-y*y/r)*V[1] );
								

                    }
                    // Store in memory
                    else if (read_from_fifo ==1 && consumer_done==0) 
                    {
							 
                        if (flag1[fifo_hash_index]==0 && fifo_index!=999999999)
                        {
                            data1[fifo_hash_index] =fifo_data; 
                            index1[fifo_hash_index] =fifo_index;
                            flag1[fifo_hash_index]=1;
                            read_from_fifo=1;									 
                            xil_printf("»»»»»»»»»»»»»»»»»%d STORED in T1 \n\r",fifo_index);
                        }
                        else if (flag2[fifo_hash_index]==0 && fifo_index!=999999999)
                        {
                            data2[fifo_hash_index] =fifo_data;
                            index2[fifo_hash_index] =fifo_index;						  
                            flag2[fifo_hash_index]=1;
                            read_from_fifo=1;
                            xil_printf("»»»»»»»»»»»»»»»»»%d STORED in T2 \n\r",fifo_index);
                        }
                        else if (flag3[fifo_hash_index]==0 && fifo_index!=999999999)
                        {
                            data3[fifo_hash_index] =fifo_data;
                            index3[fifo_hash_index] =fifo_index;
                            flag3[fifo_hash_index]=1;
                            read_from_fifo=1;
                            xil_printf("»»»»»»»»»»»»»»»»»%d STORED in T3 \n\r",fifo_index);
                        }
                        else
                        read_from_fifo=0;
							
                        
                    } // Store in memory						  
						  else
						  {
							xil_printf("ERROR ! Index %d cannot be found !!!!\n\r",consumer_index);
							consumer_done=0;
							consumer_index++;
							}
                
            
        }
    }
    xil_printf ("consumer_index =%d \t ci=%d \n\r",consumer_index,ci);
    end = XTmrCtr_GetValue(&timer, TIMER_COUNTER_0);
    XTmrCtr_Stop(&timer, TIMER_COUNTER_0);
    xil_printf("Timer Start value = %d    Timer end value = %d \r\n", start, end-start);
    return 0;
}


