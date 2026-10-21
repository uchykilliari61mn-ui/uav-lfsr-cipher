#ifndef LFSR_H
#define LFSR_H

#include <cstdint>
#include <vector>
#include <numeric>

// --------------------------------------------------------------------------
// 16-бітний LFSR (Linear Feedback Shift Register), Фібоначчі-форма.
//
// Відводи 16, 14, 13, 11 — канонічний поліном з прикладу у Вікіпедії.
// Саме цей LFSR відповідає seed'у 44257 (= 0xACE1), використаному у статті.
//
//   feedback = b0 XOR b2 XOR b3 XOR b5
//   lfsr     = (lfsr >> 1) | (feedback << 15)
//
// Seed грає роль ключа: він повністю визначає псевдовипадкову послідовність,
// а отже й «малюнок» перемішування блоків.
// --------------------------------------------------------------------------
class Lfsr16
{
public:
    explicit Lfsr16(uint16_t seed)
        : state_(seed == 0 ? 0xACE1u : seed) // 0 — заборонений стан LFSR
    {
    }

    // Один такт: повертає біт, що вийшов, і зсуває регістр.
    uint16_t nextState()
    {
        uint16_t bit = ((state_ >> 0) ^ (state_ >> 2) ^
                        (state_ >> 3) ^ (state_ >> 5)) & 1u;
        state_ = (uint16_t)((state_ >> 1) | (bit << 15));
        return state_;
    }

    // Псевдовипадкове число в діапазоні [0, bound).
    uint32_t nextInRange(uint32_t bound)
    {
        return nextState() % bound;
    }

private:
    uint16_t state_;
};

// --------------------------------------------------------------------------
// Побудова таблиці перестановок (permutation table) для n блоків.
//
// Використовуємо тасування Фішера–Єйтса, де джерелом випадковості є LFSR.
// Отримана таблиця — це бієкція 0..n-1 -> 0..n-1, тобто повна перестановка
// без колізій, повністю відтворювана за тим самим seed (ключем).
//
//   perm[i] = нова позиція блоку i
// --------------------------------------------------------------------------
inline std::vector<int> buildPermutation(int n, uint16_t seed)
{
    std::vector<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0); // 0,1,2,...,n-1

    Lfsr16 lfsr(seed);
    for (int i = n - 1; i > 0; --i)
    {
        int j = (int)lfsr.nextInRange((uint32_t)(i + 1));
        std::swap(perm[i], perm[j]);
    }
    return perm;
}

#endif // LFSR_H
