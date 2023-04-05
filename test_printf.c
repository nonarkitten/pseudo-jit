/*
 * printf.c
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 */

#include <stdio.h>

void test_printf(void) {
   /*
    * this should display (on 32bit int machine) :
    *
    * Hello world!
    * printf test
    * (null) is null pointer
    * 5 = 5
    * -2147483647 = - max int
    * char a = 'a'
    * hex ff = ff
    * hex 00 = 00
    * signed -3 = unsigned 4294967293 = hex fffffffd
    * 0 message(s)
    * 0 message(s) with %
    * justif: "left      "
    * justif: "     right"
    *  3: 0003 zero padded
    *  3: 3    left justif.
    *  3:    3 right justif.
    * -3: -003 zero padded
    * -3: -3   left justif.
    * -3:   -3 right justif.
    */    
	char *ptr = "Hello world!";
//	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	char buf[80];

	mi = (1 << (bs-1)) + 1;
	printf("%s\n", ptr);
	printf("printf test\n");
//	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
    printf("hex %08lx = ffffffff\n", -1ul);
    printf("hex %04hx = ffff\n", -1);
    printf("hex %02hhx = ff\n", -1);
    printf("hex %02lx = ffffffff\n", -1ul);
    printf("hex %02hx = ffff\n", -1);
    printf("hex %01hhx = ff\n", -1);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
    printf("float %0.3f = 0.000\n", 0.0);
    printf("float %0.2f = 3.14\n", 3.141593);
	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);
}



