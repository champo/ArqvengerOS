#include "system/cache/cache.h"
#include "system/mm/allocator.h"
#include "drivers/ata.h"
#include "library/string.h"

static struct CacheTable table;

int cache_read(unsigned long long sector, int count, void* buffer) {

    char* buff = (char*) buffer;
    struct Chunk* chunk;
    int startBlock = sector / 8;
    int endBlock = (sector + count) / 8;
    int counter = 0;

    for (int block = startBlock; block < endBlock; block++) {
        
        if ((chunk = find_chunk(block)) == NULL) {
        
            chunk = load_chunk(block);
            if (chunk == NULL) {
                return -1;
            }
        }
       
        chunk->accesses++;

        if (block == startBlock) {

            int first = sector % 8;
            int last = count > 8? 8 : count;

            memcpy(buff + (counter * SECTOR_SIZE), ((char*)chunk->buffer) + (first * SECTOR_SIZE), SECTOR_SIZE * (last - first));
            counter += (last - first);

        } else if (block == endBlock) {
            
            int last = (count - (sector % 8)) % 8;
            memcpy(buff + (counter * SECTOR_SIZE), chunk->buffer, SECTOR_SIZE * last);
            counter += last;
        } else {

            memcpy(buff + (counter * SECTOR_SIZE), chunk->buffer, SECTOR_SIZE * 8);
            counter += 8;
        }
        
    }

    return 0;
}


int cache_write(unsigned long long sector, int count, void* buffer) {

    char* buff = (char*) buffer;
    struct Chunk* chunk;
    int startBlock = sector / 8;
    int endBlock = (sector + count) / 8;
    int counter = 0;

    for (int block = startBlock; block < endBlock; block++) {
        
        if ((chunk = find_chunk(block)) == NULL) {
        
            chunk = load_chunk(block);
            if (chunk == NULL) {
                return -1;
            }
        }

        chunk->dirty = 1;
        chunk->accesses++;
    
        
        if (block == startBlock) {

            int first = sector % 8;
            int last = count > 8? 8 : count;

            memcpy(((char*)chunk->buffer) + first * SECTOR_SIZE, buff + (counter * SECTOR_SIZE), SECTOR_SIZE * (last - first));
            counter += (last - first);

        } else if (block == endBlock) {
            
            int last = (count - (sector % 8)) % 8;
            memcpy(chunk->buffer, buff + (counter * SECTOR_SIZE), SECTOR_SIZE * last);
            counter += last;
        } else {

            memcpy(chunk->buffer, buff + (counter * SECTOR_SIZE), SECTOR_SIZE * 8);
            counter += 8;
        }
        
    }

    //write-trough
    ata_write(sector, count, buffer);

    return 0;
}

