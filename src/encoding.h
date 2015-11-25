#ifndef ENCODING_H
#define ENCODING_H

/* 	@function			Encode in 16 bytes a sequence of length <= 32 of chars representing bases.
**
**	@param	enc			Pointer to 16 bytes of memory to store encoding.
**	@param 	seq			The sequence to be encoded.
**	@param 	n			Number of bases in sequence. Must be <= 32.
**	@param	err_c		Pointer for recording an unencodable character.
**	@param	err_i		Pointer for recording index in sequence of unencodable character.
**
**	@return 			1 if error occured, otherwise 0.
*/
int encode_seq(unsigned char* enc, const char* seq, int n, int* err_c, int* err_i);

/* 	@function			Decode an encoded sequence of bases.
**
**	@param	seq			Pointer to n bytes of memory to store decoded sequence.
**	@param 	enc			The encoded sequence.
**	@param 	n			Number of bases in sequence. Must be <= 32.
*/
void decode_seq(char* seq, unsigned char* enc, int n);

/* 	@function			Update an encoded base sequence by adding a base on the end and removing one from the start.
**
**	@param	enc			16 byte encoding to be updated.
**	@param	b			Character representing a base to be added onto the end of the encoded sequence.
**	@param 	n			Number of bases in sequence. Must be <= 32.
**
**	@return 			1 if error occured, otherwise 0.
*/
int encode_update(unsigned char* enc, char b,  int n);

#endif