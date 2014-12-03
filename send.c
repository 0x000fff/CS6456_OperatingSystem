#define true  1
#define false 0

void main()
{
    char msg[100];
    enableInterrupts();
    msg[0] = 'H';
    msg[1] = 'e';
    msg[2] = 'l';
    msg[3] = 'l';
    msg[4] = 'o';
    msg[5] = ' ';
    msg[6] = 'w';
    msg[7] = 'o';
    msg[8] = 'r';
    msg[9] = 'l';
    msg[10] = 'd';
    msg[11] = '!';
    msg[12] = 0xa;
    msg[13] = 0xd;
    msg[14] = 0;

    interrupt(0x21, 11, msg, 1, 0);
    interrupt(0x21, 0, "(send) msg sent.\n\r", 0, 0);
    interrupt(0x21, 0, "(send) terminating.\n\r", 0, 0);
    //interrupt(0x21, 0, msg, 0, 0);
    interrupt(0x21, 0, "\n\rSHELL> ", 0, 0);
    interrupt(0x21, 5, 0, 0, 0);
}
