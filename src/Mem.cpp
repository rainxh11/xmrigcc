/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <memory.h>

#include "crypto/CryptoNight.h"
#include "Mem.h"

bool Mem::m_useHugePages = true;
size_t Mem::m_hashFactor = 1;
int Mem::m_flags         = 0;
int Mem::m_totalPages = 0;
int Mem::m_totalHugepages = 0;
Options::Algo Mem::m_algo = Options::ALGO_CRYPTONIGHT;
Mem::ThreadBitSet Mem::m_multiHashThreadMask = Mem::ThreadBitSet(-1L);

ScratchPadMem Mem::create(ScratchPad** scratchPads, int threadId)
{
    size_t scratchPadSize;

    switch (m_algo) {
        case Options::ALGO_CRYPTONIGHT_LITE:
            scratchPadSize = MEMORY_LITE;
            break;
        case Options::ALGO_CRYPTONIGHT_HEAVY:
            scratchPadSize = MEMORY_HEAVY;
            break;
        case Options::ALGO_CRYPTONIGHT:
        default:
            scratchPadSize = MEMORY;
            break;
    }

    ScratchPadMem scratchPadMem;
    scratchPadMem.realSize = scratchPadSize * getThreadHashFactor(threadId);
    scratchPadMem.size = scratchPadSize * getThreadHashFactor(threadId);
    scratchPadMem.size += scratchPadMem.size % MEMORY;
    scratchPadMem.pages = scratchPadMem.size / MEMORY;

    allocate(scratchPadMem, m_useHugePages);

    for (size_t i = 0; i < getThreadHashFactor(threadId); ++i) {
        ScratchPad* scratchPad = static_cast<ScratchPad *>(_mm_malloc(sizeof(ScratchPad), 4096));
        scratchPad->memory     = scratchPadMem.memory + (i * scratchPadSize);

        scratchPads[i] = scratchPad;
    }

    m_totalPages += scratchPadMem.pages;
    m_totalHugepages += scratchPadMem.hugePages;

    return scratchPadMem;
}

void Mem::release(ScratchPad** scratchPads, ScratchPadMem& scratchPadMem, int threadId)
{
    m_totalPages -= scratchPadMem.pages;
    m_totalHugepages -= scratchPadMem.hugePages;

    release(scratchPadMem);

    for (size_t i = 0; i < getThreadHashFactor(threadId); ++i) {
        _mm_free(scratchPads[i]);
    }
}