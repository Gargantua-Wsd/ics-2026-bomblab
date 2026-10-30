#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
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
    (void)challenge;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t low  = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid  = (uint32_t)(challenge >> 16);
    uint32_t a = (low * 5U + high) ^ 46669U;
    uint32_t b = (mid + 7351U) ^ high;
    uint32_t c = (b + low * 8U) ^ 42398U;
    values[0] = c ^ a;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = (uint32_t)((challenge >> 5) & 31U) * 3U;
    for (unsigned int i = 1; i < 6; ++i) {
        unsigned int shift = 16U + 5U * (i - 1U);
        uint32_t part = (uint32_t)((challenge >> shift) & 63U);
        values[i] = ((values[i - 1] << 2) ^ part) + 9U * i;
    }
    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t k = challenge & 7U;
    uint32_t t = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    uint32_t result = 0;
    switch (k){
        case 0:
            result = (t ^ 1601U) + (t >> 1);
            break;
        case 1:
            result = (t ^ 3877U) + (t >> 1);
            break;
        case 2:
            result = (t * 2U) ^ 13664U;
            break;
        case 3:
            result = t * 5U + 14559U;
            break;
        case 4:
            result = t * 3U + 48450U;
            break;
        case 5:
            result = (t * 4U) ^ 38510U;
            break;
        case 6:
            result = t * 5U + 189135U;
            break;
        case 7:
            result = t * 5U + 68460U;
            break;
    }
    values[0] = k;
    values[1] = result;
    count = 2;
    return write_answer(out, cap, values, count);
}



static uint32_t solve_p4_walk(uint32_t target, uint32_t low,
                              uint32_t high, uint32_t key) {
    uint32_t mid = low + (high - low) / 2U;
    if (target == mid) {
        return (mid + 128U) ^ key;
    }
    uint32_t saved = mid + key;
    uint32_t child;
    if (target < mid) {
        child = solve_p4_walk(target, low, mid - 1U, key);
        return (child * 5U) ^ saved ^ 29U;
    } else {
        child = solve_p4_walk(target, mid + 1U, high, key);
        return (child * 5U) ^ saved ^ 20U;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t target = (uint32_t)challenge & 127U;
    uint32_t key = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = solve_p4_walk(target, 0U, 127U, key);
    count = 1;
    return write_answer(out, cap, values, count);
}




int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    /*每个节点的两个字段*/
    static const uint32_t a[12] = {
        20793, 31871, 23266, 8250,
        19500, 1943, 48114, 52941,
        440, 56721, 13902, 7528
    };

    static const uint32_t b[12] = {
        2416, 1920, 2910, 44,
        1148, 2609, 2071, 526,
        1410, 881, 324, 1558
    };

    /*-1表示空指针*/
    static const int next[12] = {
        5, 4, 1, 2, -1, 8,
        9, 10, 3, 11, 0, 7
    };

    static const int heads[8] = {
        6, 9, 11, 7, 10, 0, 5, 8
    };

    uint32_t index = (uint32_t)challenge & 7U;
    uint32_t steps = (uint32_t)(challenge >> 3) & 3U;

    uint32_t state = (uint32_t)(challenge >> 6)
                   ^ (uint32_t)(challenge >> 32)
                   ^ 8670U;

    int node = heads[index];

    /*处理起始节点，然后再沿 next 移动 steps 次*/
    for (uint32_t i = 0; i <= steps; ++i) {
        state = (state * 3U + a[node]) ^ b[node];
        values[1] = b[node];

        if (i < steps) {
            node = next[node];
        }
    }

    values[0] = state;

    return write_answer(out, cap, values, count);
}



static uint32_t solve_p6_op(uint32_t index, uint32_t x) {
    switch (index) {
        case 0:
        case 2:
            return ((x << 2) | (x >> 30)) ^ 28937U;
        case 1:
            return ((x >> 7) ^ x) + 35792U;
        case 3:
        case 6:
            return (x + 55498U) ^ (x >> 7);
        case 4:
            return ((x << 5) + 42170U) ^ (x * 7U);
        case 5:
            return (x ^ 7950U) + x * 4U;
        case 7:
            return (x >> 2) | (x << 30);
        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t x = (uint32_t)(challenge >> 6)
           ^ (uint32_t)(challenge >> 32);
    uint32_t index1 = (uint32_t)challenge & 7U;
    uint32_t index2 = (uint32_t)(challenge >> 3) & 7U;  
    uint32_t first = solve_p6_op(index1, x);
    uint32_t y = first ^ 45667U;
    uint32_t second = solve_p6_op(index2, y);
    values[0] = second + 23276U;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    uint32_t a = (uint32_t)context & 3U;
    uint32_t u = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t v = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t w = (a * 1776U) ^ (uint32_t)(context >> 16);
    uint32_t x = u ^ v;
    uint32_t mixed = ((x >> 5) | (x << 27)) ^ 8744U;
    uint32_t b = mixed ^ w;
    uint32_t c = b ^ (v + a + 9432U + (u >> 5));
    int size = snprintf(out, cap,"AAAAAAAAAAAAAAAA""AAAAAAAAAAAAAAAA""OPEN %" PRIu32 " %" PRIu32 " %" PRIu32,a, b, c);
return size < 0 || (size_t)size >= cap;
}
