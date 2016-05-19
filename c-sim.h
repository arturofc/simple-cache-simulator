#ifndef C_SIM_H_
#define C_SIM_H_

struct Block* init_block();
struct Block* create_block(uint32_t, int, int, uint32_t, uint32_t);
void direct(int, FILE*, int);
void full_assoc(int, FILE*, int);
void n_assoc(int, FILE*, int, int);

#endif
