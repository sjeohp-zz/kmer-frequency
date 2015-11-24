unsigned char encode_base(char b);
char decode_base(unsigned char b);
int encode_seq(unsigned char* enc, const char* seq, int n);
int decode_seq(char* seq, unsigned char* enc, int n);
int encode_update(unsigned char* enc, char b,  int n);