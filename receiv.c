#define true  1
#define false 0

void main()
{
    char msg[100];
    enableInterrupts();

    interrupt(0x21, 12, msg, 0, 0);
    interrupt(0x21, 0, "(receiv) msg received:\n\r", 0, 0);
    interrupt(0x21, 0, msg, 0, 0);
    //interrupt(0x21, 0, "SHELL> ", 0, 0);
    interrupt(0x21, 5, 0, 0, 0);
}
