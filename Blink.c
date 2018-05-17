/*//Blink
#include <xtimer.h>
#include<stdio.h>
#include <periph/gpio.h>
#include <pthread.h>
//#include <mutex.h>

kernel_pid_t scan_inputs;

//mutex_t mutex_lock;

pthread_mutex_t mutex_lock;


gpio_t output_LED = GPIO_PIN(PORT_B,7);   //Digital Pin 13
gpio_t Input_A    = GPIO_PIN(PORT_B,6);   //Digital Pin 12
gpio_t Input_B    = GPIO_PIN(PORT_B,5);   //Digital Pin 11


void *AND_Gate_Inputs(void* arg)
{
	(void)arg;
	printf("I'm observing inputs")
}

thread_t t[3];

void main()
{
	msg_t Initiate_Process;
	Initiate_Process=Initiate_Process;
	mutex_init(&mutex_lock);

	scan_inputs = thread_create (&t[0], NULL, AND_Gate_Inputs, NULL);

	pthread_exit(NULL);
	//while (1) 
	{
	
	}


}

*/

/*
gpio_t blink_pin = GPIO_PIN(PORT_B,7);

int main(void)
{

	//gpio_init( blink_pin , GPIO_OUT);


	while(1)
	{
		gpio_write(blink_pin, 0);
		xtimer_usleep(100000);
		gpio_write(blink_pin, 0);
		xtimer_usleep(100000);
	}

	return 0;
}

*/
/*

#include <stdio.h>
#include <thread.h>
#include <mutex.h>
#include <xtimer.h>
#include <periph/gpio.h>
// port b => pins 19, 20 & 22
gpio_t pin22 = GPIO_PIN(PORT_A, 0);
gpio_t pin24 = GPIO_PIN(PORT_A, 2);
gpio_t pin26 = GPIO_PIN(PORT_A, 4);
int main(void)
{
gpio_init( pin22 , GPIO_IN_PU );
gpio_init( pin24 , GPIO_IN_PU );
gpio_init( pin26 , GPIO_OUT);
while(1){
printf("Dude\n");
if((gpio_read(pin22) > 0) && (gpio_read(pin24) > 0))
{
gpio_write(pin26, 0);
printf("LOW\n");
}
if((gpio_read(pin22) == 0) && (gpio_read(pin24) == 0))
{
gpio_write(pin26, 1);
printf("High\n");
}
}
return 0;
}*/








/////////////////////////////////////////



#include <stdio.h>
#include <thread.h>
#include <xtimer.h>
#include <periph/gpio.h>
#include <mutex.h>
#include <msg.h>
kernel_pid_t Scan_Inputs;
kernel_pid_t Compute_AND_Gate;
kernel_pid_t Result_of_AND_Gate;
mutex_t lock;
gpio_t First_Input_Pin = GPIO_PIN(PORT_B,5); // Aurduino pin 11
gpio_t Second_Input_Pin = GPIO_PIN(PORT_B,6); // Aurduino pin 12
gpio_t Output_pin = GPIO_PIN(PORT_B,7); //Aurduino pin 13
void * Scan_AND_Inputs(void* arg)
{
(void)arg;
gpio_init(First_Input_Pin,GPIO_IN_PU);
gpio_init(Second_Input_Pin,GPIO_IN_PU);
int AND_Gate_Inputs[2];
msg_t Incoming_message, Outgoing_message;
while(1)
{
msg_receive(&Incoming_message); 
mutex_lock (&lock); 
if(gpio_read(First_Input_Pin) == 0)
AND_Gate_Inputs[0] = 0;
else AND_Gate_Inputs[0] = 1;
if(gpio_read(Second_Input_Pin) == 0)
AND_Gate_Inputs[1] = 0;
else AND_Gate_Inputs[1] = 1;
Outgoing_message.content.ptr=AND_Gate_Inputs;
mutex_unlock (&lock); 
xtimer_usleep(1000); 
msg_send(&Outgoing_message, Compute_AND_Gate); 
}
}
void * Processing_AND(void* arg)
{
(void)arg;
msg_t Incoming_message, Outgoing_message;
while(1)
{
msg_receive(&Incoming_message);
mutex_lock (&lock);
Outgoing_message.content.value = *(int *)Incoming_message.content.ptr & *((int*)Incoming_message.content.ptr+1); 

mutex_unlock (&lock);
xtimer_usleep(250); 
msg_send(&Outgoing_message, Result_of_AND_Gate); 
}
}
void * AND_Gate_Output(void* arg)
{
(void)arg;

msg_t Incoming_message;
gpio_init(Output_pin,GPIO_OUT); 
while(1)
{
msg_receive(&Incoming_message); 
mutex_lock (&lock);
if(Incoming_message.content.value == 1)
gpio_set(Output_pin);
else
gpio_clear(Output_pin);
mutex_unlock (&lock);
xtimer_usleep(1000);
}
}
char t1_stack[THREAD_STACKSIZE_MAIN];
char t2_stack[THREAD_STACKSIZE_MAIN];
char t3_stack[THREAD_STACKSIZE_MAIN];
int main(void)
{
msg_t Outgoing_message;
mutex_init(&lock); 
Scan_Inputs = thread_create (t1_stack, sizeof(t1_stack), THREAD_PRIORITY_MAIN - 1,THREAD_CREATE_WOUT_YIELD, Scan_AND_Inputs, NULL, NULL);
Compute_AND_Gate = thread_create (t2_stack, sizeof(t2_stack), THREAD_PRIORITY_MAIN - 1,THREAD_CREATE_WOUT_YIELD, Processing_AND, NULL, NULL);
Result_of_AND_Gate = thread_create (t3_stack, sizeof(t3_stack), THREAD_PRIORITY_MAIN - 1,THREAD_CREATE_WOUT_YIELD, AND_Gate_Output, NULL, NULL);
while (1) {
msg_send(&Outgoing_message, Scan_Inputs); 
}
return 0;
}






///Barne
/*

#include <stdio.h>
#include <thread.h>
#include <xtimer.h>
#include <periph/gpio.h>
#include <mutex.h>
#include <msg.h>
//Gloabal thread IDs since thread send messages to each other
kernel_pid_t Inputs_id;
kernel_pid_t Process_id;
kernel_pid_t Output_id;
//Global declaration of mutex, since all threads are using the same
mutex_t busy;
//Global declaration of pins from Port_A (pin 22, 24, 26 on the board)
gpio_t A = GPIO_PIN(PORT_A,0);
gpio_t B = GPIO_PIN(PORT_A,2);
gpio_t C = GPIO_PIN(PORT_A,4);
//First thread body
void * AND_Inputs(void* arg)
{
// to avoid compiler error
(void)arg;
//Initiasing pin A and B as input pins with pull up
gpio_init(A,GPIO_IN_PU);
gpio_init(B,GPIO_IN_PU);
//Array which stores Low/High from input pins
int input[2];
//Declaration of messages
msg_t msg_in, msg_out;

while(1)
{
msg_receive(&msg_in); //waits for a message
mutex_lock (&busy); //Mutex locks thread
//input array is set true or false depending of input
if(gpio_read(A) != 0)
input[0] = 1;
else input[0] = 0;
if(gpio_read(B) != 0)
input[1] = 1;
else input[1] = 0;
//content pointer of out message points to base address of array
msg_out.content.ptr=input;
mutex_unlock (&busy); //release mutex
xtimer_usleep(1000); //sleep for 1ms
msg_send(&msg_out, Process_id); //send out message to next thread
}
}
//second thread body
void * AND_Process(void* arg)
{
(void)arg;
msg_t msg_in, msg_out;
while(1)
{
msg_receive(&msg_in);//waits for a message
mutex_lock (&busy);
msg_out.content.value = *(int *)msg_in.content.ptr & *((int
*)msg_in.content.ptr+1); //compares value in array addresses
// and stores output in next output message
mutex_unlock (&busy);
xtimer_usleep(250); //simulated processing time
msg_send(&msg_out, Output_id); //send output message to Output thread
}
}
//third thread body
void * AND_Output(void* arg)
{
(void)arg;

msg_t msg_in;
gpio_init(C,GPIO_OUT); //declares pin C as output pin
while(1)
{
msg_receive(&msg_in); //waits for a message
mutex_lock (&busy);
//set output pin high if contnent of in pu message is true otherwise low
if(msg_in.content.value == 1)
gpio_set(C);
else
gpio_clear(C);
mutex_unlock (&busy);
xtimer_usleep(1000);
}
}
//declaration of stack sizes
char t1_stack[THREAD_STACKSIZE_MAIN];
char t2_stack[THREAD_STACKSIZE_MAIN];
char t3_stack[THREAD_STACKSIZE_MAIN];
int main(void)
{
msg_t msg_out;
mutex_init(&busy); //initialize mutex
//threads creation
Inputs_id = thread_create (t1_stack, sizeof(t1_stack), THREAD_PRIORITY_MAIN - 1,
THREAD_CREATE_WOUT_YIELD, AND_Inputs, NULL, NULL);
Process_id = thread_create (t2_stack, sizeof(t2_stack), THREAD_PRIORITY_MAIN - 1,
THREAD_CREATE_WOUT_YIELD, AND_Process, NULL, NULL);
Output_id = thread_create (t3_stack, sizeof(t3_stack), THREAD_PRIORITY_MAIN - 1,
THREAD_CREATE_WOUT_YIELD, AND_Output, NULL, NULL);
while (1) {
msg_send(&msg_out, Inputs_id); //sends message to input thread in a loop to wake it up
}
return 0;
}



*/
