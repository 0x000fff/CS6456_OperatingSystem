void main()
{
    int i, m;
    int silly=0;
    char CRLF[3];
    enableInterrupts();

    CRLF[0] = 0xa;
    CRLF[1] = 0xd;
    CRLF[2] = 0;
    for (i=0;i<1000;++i){
        for(m=0;m<3000;++m) {
            silly +=m;
            if (silly > 100) silly =0;
        }
    }
    interrupt(0x21, 0, "Hi! Process #", 0, 0);
    interrupt(0x21, 17, 0, 0, 0);
    interrupt(0x21, 0, " current directory = ", 0, 0);
    interrupt(0x21, 14, 0, 0, 0);
    interrupt(0x21, 0, CRLF, 0, 0);
}
