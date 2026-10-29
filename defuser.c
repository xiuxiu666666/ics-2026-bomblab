#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8





static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t high32 = (uint32_t)(challenge >> 32); 
    uint32_t low32  = (uint32_t)challenge;         
    uint32_t mid32  = (uint32_t)(challenge >> 16); 
    uint32_t step1_ret = (high32 * 3) ^ (mid32 + 0xfdd);
    uint32_t step0_ret = (high32 ^ 0x553) + (low32 * 8);
    uint32_t final_ans = (step0_ret + step1_ret * 8) ^ 0x7245;

    
    values[0] = final_ans;
    count = 1; 

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    values[0] = (uint32_t)(((challenge >> 8) & 0x7F) * 3);

    for (int i = 1; i <= 5; i++) {
        int shift = 11 + (i - 1) * 3;
        uint32_t chunk = (uint32_t)((challenge >> shift) & 0xF);
        values[i] = ((chunk + i) ^ values[i - 1]) * 5;
    }

    count = 6;

    return write_answer(out, cap, values, count);
}






static uint32_t p3_dispatch_logic(uint32_t idx, uint32_t val) {
    switch (idx) {
        case 0: return (0xb391 + (val * 3));                
        case 1: return ((val * 2) ^ 0xa8f7);                  
        case 2: return ((val ^ 0x58d6) + (val >> 2));         
        case 3: return ((val ^ 0x4228) + (val >> 2));         
        case 4: return ((val * 2) ^ 0x49a9);                  
        case 5: return (0x969 + (val * 3));
        case 6: return (0x14442 + (val * 5));                 
        case 7: return (0x2c34e + (val * 5));                 
        default: return 0;
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t arg1 = (uint32_t)(challenge & 0x7);
    values[0] = arg1;

    uint32_t c_low3 = (uint32_t)(challenge >> 3);
    uint32_t c_high32 = (uint32_t)(challenge >> 32);
    uint32_t arg2 = c_low3 ^ c_high32;

    values[1] = p3_dispatch_logic(arg1, arg2);

    count = 2;
    return write_answer(out, cap, values, count);
}









static uint32_t p4_walk(uint32_t key, uint32_t left, uint32_t right, uint32_t x) {
    uint32_t mid = left + ((right - left) >> 1);
    uint32_t temp = mid + x;

    if (key == mid) {
        return (mid + 7) ^ x;
    } else if (key > mid) {
        uint32_t res = p4_walk(key, mid + 1, right, x);
        return ((res * 5) ^ temp) ^ 0x14;
    } else { 
        uint32_t res = p4_walk(key, left, mid - 1, x);
        return ((res * 5) ^ temp) ^ 0x1d;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t key = (uint32_t)(challenge & 0x7F);
    uint32_t x = (uint32_t)((challenge >> 7) ^ (challenge >> 32));

    
    values[0] = p4_walk(key, 0, 0x7F, x);
    
    count = 1;

    return write_answer(out, cap, values, count);
}







typedef struct {
    uint32_t next;
    uint32_t value;
    uint32_t salt;
} P5Node;

static const P5Node p5_nodes[] = {
    {7, 0x0b75u, 0x59d6u},
    {0, 0x0029u, 0xcc2cu},
    {6, 0x0574u, 0x5260u},
    {4, 0x0637u, 0x39cau},
    {5, 0x0361u, 0xe823u},
    {UINT32_MAX, 0x0884u, 0xf9c8u},
    {8, 0x0965u, 0x886cu},
    {2, 0x027bu, 0x45f6u},
    {3, 0x0430u, 0x60eau}
};

static const uint32_t p5_heads[8] = {
    1, 0, 7, 2, 6, 8, 3, 4
};

static uint32_t p5_traverse(uint64_t challenge, uint32_t *out)
{
    uint32_t index = p5_heads[challenge & 0x7u];
    uint32_t steps = (uint32_t)((challenge >> 3) & 0x3u);

    uint32_t result =
        (uint32_t)(challenge >> 6) ^
        (uint32_t)(challenge >> 32);

    result ^= 0xfc32u;

    for (uint32_t i = 0; i <= steps; ++i) {
        const P5Node *node = &p5_nodes[index];

        result += node->salt;
        result *= 5u;

        *out = node->value;
        result ^= node->value;

        if (i != steps) {
            index = node->next;
        }
    }

    return result;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t second_value = 0;
    uint32_t final_ans = p5_traverse(challenge, &second_value);
    fprintf(
    stderr,
    "P5 challenge=0x%016" PRIx64 " answer=%" PRIu32 " %" PRIu32 "\n",
    challenge,
    final_ans,
    second_value
);

    values[0] = final_ans;
    count = 1;

    values[count] = second_value;
    ++count;

    return write_answer(out, cap, values, count);
}




static uint32_t p6_op_0(uint32_t x) { return (x * 8 + 0xbef0) ^ (x * 3); }
static uint32_t p6_op_1(uint32_t x) { return ((x >> 2) ^ x) + 0xd43f; }
static uint32_t p6_op_2(uint32_t x) { return (x ^ 0xbf99) + (x * 4); }
static uint32_t p6_op_3(uint32_t x) { return ((x << 7) | (x >> 25)) ^ 0xdce9; }
static uint32_t p6_op_4(uint32_t x) { return (x >> 1) | (x << 31); }
static uint32_t p6_op_5(uint32_t x) { return (x + 0x3ed1) ^ (x >> 2); }

static uint32_t p6_dispatch_logic(uint32_t x, uint32_t idx) {
    switch (idx) {
        case 0: case 1: return p6_op_5(x);
        case 2: return p6_op_0(x);
        case 3: case 7: return p6_op_3(x);
        case 4: return p6_op_2(x);
        case 5: return p6_op_1(x);
        case 6: return p6_op_4(x);
        default: return 0;
    }
}


int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t arg1 = (uint32_t)((challenge >> 6) ^ (challenge >> 32));
    uint32_t idx1 = challenge & 0x7;
    
    uint32_t ret1 = p6_dispatch_logic(arg1, idx1);
    uint32_t res1 = ret1 ^ 0x89ee;

    uint32_t idx2 = (challenge >> 3) & 0x7;
    uint32_t ret2 = p6_dispatch_logic(res1, idx2);

    values[0] = ret2 + 0x663;
    count = 1;

    return write_answer(out, cap, values, count);
}



static uint32_t secret_mix(uint32_t x)
{
    uint32_t y = x ^ 0x6b82u;
    uint32_t z = x >> 1;
    z ^= 0xecd2u;
    return y + z;
}

int solve_secret(uint64_t challenge, uint64_t transcript,
                 char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t a_low = (uint32_t)challenge;
    uint32_t a_high = (uint32_t)(challenge >> 32);
    uint32_t b_low = (uint32_t)transcript;
    uint32_t b_high = (uint32_t)(transcript >> 32);

    uint32_t selector = (uint32_t)(transcript & 0x3u);
    uint32_t a_flip = a_low ^ a_high;
    uint32_t b_flip = b_low ^ b_high;

    uint32_t mixed = secret_mix(a_flip ^ b_flip);

    uint32_t field_one = selector;

    uint32_t field_two =
        mixed ^
        (uint32_t)(transcript >> 16) ^
        (selector * 0x6b82u);

    uint32_t tail =
        0xecd2u +
        b_flip +
        selector +
        (a_flip >> 1);

    uint32_t field_three = field_two ^ tail;

    values[count] = field_one;
    ++count;

    values[count] = field_two;
    ++count;

    values[count] = field_three;
    ++count;

    int prefix_length = snprintf(
        out,
        cap,
        "%s",
        "0123456789abcdef0123456789abcdefOPEN"
    );

    if (prefix_length < 0 ||
        (size_t)prefix_length >= cap) {
        return 1;
    }

    return write_answer(
        out + prefix_length,
        cap - (size_t)prefix_length,
        values,
        count
    );
}