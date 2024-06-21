//
// Created by 高昶 on 2024/6/21.
//

#ifndef IOCANARYLEARN_MD5_H
#define IOCANARYLEARN_MD5_H

#ifdef __cplusplus
extern "C" {
#endif

#define MD5_CBLOCK    64
#define MD5_LBLOCK    (MD5_CBLOCK/4)
#define MD5_SIZE 16

typedef struct {
    unsigned int A, B, C, D;
    unsigned int Nl, Nh;
    unsigned int data[MD5_LBLOCK];
    unsigned int num;
} md5_t;

void    MD5_init(md5_t*);
void    MD5_process(md5_t*, const void*, unsigned int);
void    MD5_finish(md5_t*, void*);

void    MD5_buffer(const char* buffer, const unsigned int buf_len, void* signature);

void    MD5_sig_to_string(const void* signature, char str[2 * MD5_SIZE]);
void    MD5_sig_from_string(void* signature, const char str[2 * MD5_SIZE]);

#ifdef __cplusplus
}
#endif

#endif //IOCANARYLEARN_MD5_H
