#include "system/cache/cache.h"
#include "system/mm/allocator.h"
#include "drivers/ata.h"
#include "library/string.h"

static struct CacheTable table;

int searchTable(unsigned long long sector, int count);

int cache_read(unsigned long long sector, int count, void* buffer) {

    int n;
    if ((n = searchTable(sector,count)) != -1){
        
        buffer = table.chunks[n].buffer;
    } else {
        
        ata_read(sector, count, buffer);
        
        table.chunks[table.chunksNum].initialSector = sector;
        table.chunks[table.chunksNum].sectors = count;
        table.chunks[table.chunksNum].buffer = kalloc(sector * count);
        memcpy(table.chunks[table.chunksNum].buffer, buffer, sector * count);
        table.chunksNum++;

    }

    return 0;
}


int cache_write(unsigned long long sector, int count, void* buffer) {

    int n; 
    if ((n = searchTable(sector, count)) != -1){
        
        table.chunks[n].buffer = buffer;
        table.chunks[n].dirty = 1;
        table.chunks[n].accesses++;
    } else {
        
        table.chunks[table.chunksNum].initialSector = sector;
        table.chunks[table.chunksNum].sectors = count;
        table.chunks[table.chunksNum].buffer = kalloc(sector * count);
        memcpy(table.chunks[table.chunksNum].buffer, buffer, sector * count);
        table.chunksNum++;

    }

    return 0;
}


int searchTable(unsigned long long sector, int count) {

    return 0;
}
