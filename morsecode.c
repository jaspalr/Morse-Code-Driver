/*
 * morse-code.c
 * - Uses a linux driver to display characters in morse code
 *      Author: Jaspal Raman
 */
#include <linux/module.h>
#include <linux/miscdevice.h>		// for misc-driver calls.
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
 #include <linux/leds.h>
#include <linux/kfifo.h>
#include <linux/kfifo.h>
#include <stdbool.h>
//#error Are we building this?

#define MY_DEVICE_FILE  "morse-code"
#define maskdot 1 << 15
#define maskdash ((1 << 2) | 111) << 13

// Define the toy data:
#define START_CHAR 'A'
#define END_CHAR   'z'
#define DATA_SIZE  2

DEFINE_LED_TRIGGER(my_trigger);
static DECLARE_KFIFO(my_fifo, char, 128);


static unsigned short morsecode_codes[] = {
		0xB800,	// A 1011 1
		0xEA80,	// B 1110 1010 1
		0xEBA0,	// C 1110 1011 101
		0xEA00,	// D 1110 101
		0x8000,	// E 1
		0xAE80,	// F 1010 1110 1
		0xEE80,	// G 1110 1110 1
		0xAA00,	// H 1010 101
		0xA000,	// I 101
		0xBBB8,	// J 1011 1011 1011 1
		0xEB80,	// K 1110 1011 1
		0xBA80,	// L 1011 1010 1
		0xEE00,	// M 1110 111
		0xE800,	// N 1110 1
		0xEEE0,	// O 1110 1110 111
		0xBBA0,	// P 1011 1011 101
		0xEEB8,	// Q 1110 1110 1011 1
		0xBA00,	// R 1011 101
		0xA800,	// S 1010 1
		0xE000,	// T 111
		0xAE00,	// U 1010 111
		0xAB80,	// V 1010 1011 1
		0xBB80,	// W 1011 1011 1
		0xEAE0,	// X 1110 1010 111
		0xEBB8,	// Y 1110 1011 1011 1
		0xEEA0	// Z 1110 1110 101
};




/******************************************************
 * Callbacks
 ******************************************************/


static ssize_t my_read(struct file *file,
		char *buff, size_t count, loff_t *ppos)
{

	int bytes_read = 0;

	if(kfifo_to_user(&my_fifo,buff,count, &bytes_read)){
			printk(KERN_ERR "Unable to write to buffer.");
			return -EFAULT;
	}

	// Write to in/out parameters and return:
	*ppos = bytes_read;
	return bytes_read;  // # bytes actually read.
}


static ssize_t my_write(struct file *file,
		const char *buff, size_t count, loff_t *ppos)
{
	int buff_idx = 0;

	unsigned short  val = 0;
	unsigned short bit3;
	unsigned short bit;
	char ch;
	int i = 0;
	bool word = false;
	bool dot = false;
	bool space = false;
	bool ischar = false;


	



	// Find min character
	for (buff_idx = 0; buff_idx < count; buff_idx++) {
		

		// Get the character
		if (copy_from_user(&ch, &buff[buff_idx], sizeof(ch))) {
			return -EFAULT;
		}



		if(ch == ' ' ){
		
			dot = false;
			word = false;
			space = true;
			continue;
		}
		//convent to uppercase
        ch = ch & 1110111;

        

        



		// Skip special characters:

		if((ch< '@' || ch > '[')) {
			continue;
		}

		
		if(ischar && space){//space case
			
			word = false;
			space = false;
			
			led_trigger_event(my_trigger, LED_OFF);
			
			dot = false;
			kfifo_put(&my_fifo, ' ');
			kfifo_put(&my_fifo, ' ');
			msleep(1400);
	
		}

		if(word){//if at the end
			led_trigger_event(my_trigger, LED_OFF);
			dot = false;
			word = false;
				
			kfifo_put(&my_fifo, ' ');
				
			msleep(600);
				
		}
		ischar = true;

		val = morsecode_codes[ch - 65];
		word = true;
		
		for(i = 0; i < 16; i++){



				bit3 = val & maskdash;

				bit = val & maskdot;
				
			
				//val <<= 1;
				//mask >>=1;
				//printk(KERN_INFO "bit %x,%x,%x\n",bit,val,mask);
				if(bit3 == 57344){
					if(dot){
						led_trigger_event(my_trigger, LED_OFF);
						msleep(200);
					}
					dot =true;
					led_trigger_event(my_trigger, LED_FULL);
					kfifo_put(&my_fifo, '-');
					msleep(600);

					i+=2;
					val <<= 3;
				}
				else if(bit > 0){
					if(dot){
						led_trigger_event(my_trigger, LED_OFF);
						msleep(200);
					}
					dot =true;
					led_trigger_event(my_trigger, LED_FULL);
					kfifo_put(&my_fifo, '.');
			
					
					val <<= 1;
					msleep(200);

				}
				else{
					dot = false;
					led_trigger_event(my_trigger, LED_OFF);
					if(val == 0){
						break;

					}
					else{
						val <<= 1;
						msleep(200);
		
					}
				}
				
				
				
				

	

			

		}
		led_trigger_event(my_trigger, LED_OFF);
		

        

	
	}
	kfifo_put(&my_fifo, '\n');



	// Return # bytes actually written.
	*ppos += count;
	return count;
}

/******************************************************
 * Misc support
 ******************************************************/
// Callbacks:  (structure defined in /linux/fs.h)
struct file_operations my_fops = {
	.owner    =  THIS_MODULE,
	.read     =  my_read,
	.write    =  my_write,
};

// Character Device info for the Kernel:
static struct miscdevice my_miscdevice = {
		.minor    = MISC_DYNAMIC_MINOR,         // Let the system assign one.
		.name     = MY_DEVICE_FILE,             // /dev/.... file.
		.fops     = &my_fops                    // Callback functions.
};


/******************************************************
 * Driver initialization and exit:
 ******************************************************/
static int __init my_init(void)
{
	int ret;
	printk(KERN_INFO "----> morsecode driver init(): file /dev/%s.\n", MY_DEVICE_FILE);

	// Register as a misc driver:
	ret = misc_register(&my_miscdevice);

	INIT_KFIFO(my_fifo);

	led_trigger_register_simple(MY_DEVICE_FILE, &my_trigger);


	return ret;
}

static void __exit my_exit(void)
{
	printk(KERN_INFO "<---- morsecode driver exit().\n");

	// Unregister misc driver
	led_trigger_unregister_simple(my_trigger);
	misc_deregister(&my_miscdevice);
}



module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jaspal Raman");
MODULE_DESCRIPTION("Driver to translate words to morse code");
MODULE_LICENSE("GPL");