/**
 * Mupen64 - rom.c
 * Copyright (C) 2002 Hacktarux Tillin9
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

/* This is the functions that load a rom into memory, it loads a roms
 * in multiple formats, gzipped or not. It searches the rom, in the roms
 * subdirectory or in the path specified in the path.cfg file.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <zlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h> 

#include "zip/unzip.h"
#include "bzip2/bzlib.h"

#include "md5.h"
#include "rom.h"
#include "../memory/memory.h"
#include "guifuncs.h"
#include "romcache.h"
#include "guifuncs.h"
#include "translate.h"
#include "main.h"
#include "util.h"

//Global loaded rom memory space.
unsigned char *rom;
//Global loaded rom size.
int taille_rom;
//Global loaded rom header information.
rom_header *ROM_HEADER;
rom_settings ROM_SETTINGS;

int is_valid_rom(unsigned char buffer[4])
{
    //Test if rom is a native .z64 image with header 0x80371240. [ABCD]
    if((buffer[0]==0x80)&&(buffer[1]==0x37)&&(buffer[2]==0x12)&&(buffer[3]==0x40))
        { return 1; }
    //Test if rom is a byteswapped .v64 image with header 0x37804012. [BADC]
    else if((buffer[0]==0x37)&&(buffer[1]==0x80)&&(buffer[2]==0x40)&&(buffer[3]==0x12))
        { return 1; }
    //Test if rom is a wordswapped .n64 image with header  0x40123780. [DCBA]
    else if((buffer[0]==0x40)&&(buffer[1]==0x12)&&(buffer[2]==0x37)&&(buffer[3]==0x80))
        { return 1; }
    else
        { return 0; }
}

/*
 * Open a rom.
 * Determins compression type by testing if the decompressed file has a valid rom 4 byte header.
 * Loads loadlength of the rom into rom. //
 * and byteswaps if necessary.
 */
unsigned char* load_rom(const char *filename, int *romsize, unsigned short *compressiontype, unsigned short *imagetype, int *loadlength)
{
    int i, oldvalue;
    unsigned short romread = 0;
    char temp;
    unsigned char buffer[4];
    unsigned char *localrom;

    FILE *romfile;
    BZFILE* bzromfile;
    int bzerror;
    unzFile zipromfile;
    unz_file_info fileinfo;
    char szFileName[256], szExtraField[256], szComment[256];
    gzFile *gzromfile;

    //Uncompressed roms.
    romfile=fopen(filename, "rb");
    if(romfile!=NULL)
        {
        fread(buffer, 1, 4, romfile);
        if(is_valid_rom(buffer))
            {
            *compressiontype = UNCOMPRESSED;
            fseek(romfile, 0L, SEEK_END);
            *romsize=ftell(romfile);
            fseek(romfile, 0L, SEEK_SET);
            localrom = malloc(*loadlength);
            if(localrom==NULL)
                {
                fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
                return NULL;
                }
            fread(localrom, 1, *loadlength, romfile); 
            fclose(romfile);
            romread = 1;
            }
        else
            {
            //Bzip2 roms.
            fseek(romfile, 0L, SEEK_SET);
            bzromfile = BZ2_bzReadOpen(&bzerror, romfile, 0, 0, NULL, 0);
            if(bzerror==BZ_OK)
                { BZ2_bzRead(&bzerror, bzromfile, buffer, 4); }
            if(bzerror==BZ_OK)
                {
                if(is_valid_rom(buffer))
                    {
                    *compressiontype = BZIP2_COMPRESSION;
                    fseek(romfile, 0L, SEEK_SET);
                    bzromfile = BZ2_bzReadOpen(&bzerror, romfile, 0, 0, NULL, 0);
                    *romsize=0;
                    localrom=NULL;
                    for( i = 0; bzerror==BZ_OK; ++i )
                        {
                        localrom = (char*)realloc(localrom, (i+1)*100000);
                        if(localrom==NULL)
                           {
                           fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
                           return NULL;
                           }
                        *romsize += BZ2_bzRead(&bzerror, bzromfile, localrom+(i*100000), 100000); 
                        }
                    if(bzerror==BZ_STREAM_END)
                       {
                       localrom = (char*)realloc(localrom, *loadlength);
                       romread = 1; 
                       }
                    else
                       { free(localrom); }
                    }
                }
            BZ2_bzReadClose(&bzerror, bzromfile);
            }
       }

    //Zipped roms.
    if(romread==0)
        {
        zipromfile = unzOpen(filename);
        if(zipromfile!=NULL) 
            {
            unzGoToFirstFile(zipromfile);
            //Get first valid rom in archive.
            do
                {
                unzGetCurrentFileInfo(zipromfile, &fileinfo, szFileName, 255, 
                szExtraField, 255, szComment, 255);
                unzOpenCurrentFile(zipromfile);
                if(fileinfo.uncompressed_size>=4)
                   {
                   unzReadCurrentFile(zipromfile, buffer, 4);
                   if(is_valid_rom(buffer))
                       { 
                       *compressiontype = ZIP_COMPRESSION;
                       *romsize = fileinfo.uncompressed_size;
                       localrom = malloc(*loadlength);
                       if(localrom==NULL)
                           {
                           fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
                           return NULL;
                           }
                       unzOpenCurrentFile(zipromfile);
                       unzReadCurrentFile(zipromfile, localrom, *loadlength);
                       unzCloseCurrentFile(zipromfile);
                       romread = 1;
                       break; //Prevent error from .zips with multiple roms.
                       }
                   }
                }
            while (unzGoToNextFile(zipromfile) != UNZ_END_OF_LIST_OF_FILE);
            unzClose(zipromfile);
            }
        }

    //Gzipped roms.
    if(romread==0)
        { 
        gzromfile=gzopen(filename, "rb");
        if(gzromfile!=NULL)
            {
            gzread(gzromfile, buffer, 4);
            if(is_valid_rom(buffer))
                {
                *compressiontype = GZIP_COMPRESSION;
                gzseek(gzromfile, 0L, SEEK_SET);
                *romsize=0;
                localrom=NULL;
                for( i = 0; !gzeof(gzromfile); ++i )
                    {
                    localrom = (char*)realloc(localrom, (i+1)*100000);
                    if(localrom==NULL)
                       {
                       fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
                       return NULL;
                       }
                    *romsize += gzread(gzromfile, localrom+(i*100000), 100000);
                    }
                if(gzeof(gzromfile))
                       {
                       localrom = (char*)realloc(localrom, *loadlength);
                       romread = 1; 
                       }
                    else
                       { free(localrom); }
                gzclose(gzromfile);
                }
            }
        }

    //File invalid, or valid rom not found in file.
    if(romread==0)
        { 
        romsize = 0;
        return NULL;
        }

    //Btyeswap if .v64 image.
    if(localrom[0]==0x37)
        {
        *imagetype = V64IMAGE;
        for ( i = 0; i < (*loadlength/2); ++i )
            {
            temp=localrom[i*2];
            localrom[i*2]=localrom[i*2+1];
            localrom[i*2+1]=temp;
            }
        }
    //Wordswap if .n64 image.
    else if(localrom[0]==0x40)
        {
        *imagetype = N64IMAGE;
        for ( i = 0; i < (*loadlength/4); ++i )
            {
            temp=localrom[i*4];
            localrom[i*4]=localrom[i*4+3];
            localrom[i*4+3]=temp;
            temp=localrom[i*4+1];
            localrom[i*4+1]=localrom[i*4+2];
            localrom[i*4+2]=temp;
            }
        }
    else
        { *imagetype = Z64IMAGE; }

    return localrom;
}

static int ask_bad(void)
{
    if(g_Noask)
    {
        printf(tr("The rom you are trying to load is probably a bad dump!\n"
                      "Be warned that this will probably give unexpected results.\n"));
        return 1;
    }
    return confirm_message(tr("The rom you are trying to load is probably a bad dump!"
                           " Be warned that this will probably give unexpected results."
                           " Do you still want to run it?"));
}

static int ask_hack(void)
{
    if(g_Noask)
        {
        printf(tr("The rom you are trying to load is probably a hack!\n"
                  "Be warned that this will probably give unexpected results.\n"));
        return 1;
        }
    return confirm_message(tr("The rom you are trying to load is probably a hack!"
                           " Be warned that this will probably give unexpected results."
                           " Do you still want to run it?"));
}

int rom_read(const char *filename)
{
    md5_state_t state;
    md5_byte_t digest[16];
    romdatabase_entry *entry;
    char buffer[PATH_MAX], *s;
    char image[16], compression[16];
    unsigned short compressiontype, imagetype;
    int i;

    if(rom)
        { free(rom); }

    strncpy(buffer, filename, PATH_MAX-1);
    buffer[PATH_MAX-1] = 0;
    if ((rom=load_rom(filename, &taille_rom, &compressiontype, &imagetype, &taille_rom))==NULL)
        {
        printf ("File not found or wrong path.\n");
        return -1;
        }

    compressionstring(compressiontype, compression);
    printf("Compression: %s\n", compression);

    imagestring(imagetype, image);
    printf("Imagetype: %s\n", image);

    printf("Rom size: %d bytes (or %d Mb or %d Megabits)\n",
    taille_rom, taille_rom/1024/1024, taille_rom/1024/1024*8);

    // loading rom settings and checking if it's a good dump
    md5_init(&state);
    md5_append(&state, (const md5_byte_t *)rom, taille_rom);
    md5_finish(&state, digest);
    for ( i = 0; i < 16; ++i ) 
        { sprintf(buffer+i*2, "%02X", digest[i]); }
    buffer[32] = '\0';
    strcpy(ROM_SETTINGS.MD5, buffer);
    printf("MD5: %s", buffer);

    if(ROM_HEADER)
        { free(ROM_HEADER); }
    ROM_HEADER = malloc(sizeof(rom_header));
    if(ROM_HEADER==NULL)
        {
        fprintf( stderr, "%s, %c: Out of memory!\n", __FILE__, __LINE__ );
        return 0;
        }
    memcpy(ROM_HEADER, rom, sizeof(rom_header));
    trim((char *)ROM_HEADER->nom); // remove trailing whitespace from Rom name

    printf("%x %x %x %x\n", ROM_HEADER->init_PI_BSB_DOM1_LAT_REG,
                            ROM_HEADER->init_PI_BSB_DOM1_PGS_REG,
                            ROM_HEADER->init_PI_BSB_DOM1_PWD_REG,
                            ROM_HEADER->init_PI_BSB_DOM1_PGS_REG2);
    printf("ClockRate = %x\n", sl((unsigned int)ROM_HEADER->ClockRate));
    printf("Version: %x\n", sl((unsigned int)ROM_HEADER->Release));
    printf("CRC: %x %x\n", sl((unsigned int)ROM_HEADER->CRC1), sl((unsigned int)ROM_HEADER->CRC2));
    printf ("Name: %s\n", ROM_HEADER->nom);
    if(sl(ROM_HEADER->Manufacturer_ID) == 'N')
        { printf ("Manufacturer: Nintendo\n"); }
    else
        { printf("Manufacturer: %x\n", (unsigned int)(ROM_HEADER->Manufacturer_ID)); }
    printf("Cartridge_ID: %x\n", ROM_HEADER->Cartridge_ID);

   char country[32];
   countrycodestring(ROM_HEADER->Country_code, buffer);
   printf("Country: %s\n", buffer);
   printf ("PC = %x\n", sl((unsigned int)ROM_HEADER->PC));

    if((entry=ini_search_by_md5(digest))==&empty_entry)
        {
        char mycrc[1024];
        printf("%lx\n", (long)entry);
        if((entry=ini_search_by_crc(sl(ROM_HEADER->CRC1),sl(ROM_HEADER->CRC2)))==NULL)
            {
            strcpy(ROM_SETTINGS.goodname, (char *) ROM_HEADER->nom);
            strcat(ROM_SETTINGS.goodname, " (unknown rom)");
            printf("%s\n", ROM_SETTINGS.goodname);
            ROM_SETTINGS.eeprom_16kb = 0;
            return 0;
            }
        else
            {
            if(!ask_bad())
                {
                free(rom);
                rom = NULL;
                free(ROM_HEADER);
                ROM_HEADER = NULL;
                return -3;
                }
            strcpy(ROM_SETTINGS.goodname, entry->goodname);
            strcat(ROM_SETTINGS.goodname, " (bad dump)");
            if(entry->savetype==EEPROM_16KB)
                { ROM_SETTINGS.eeprom_16kb = 1; }
            return 0;
            }
        }

    s=entry->goodname;
    for ( i = strlen(s); i > 0 && s[i-1] != '['; --i );
    if(i!=0)
           {
       if(s[i]=='T'||s[i]=='t'||s[i]=='h'||s[i]=='f'||s[i]=='o')
           {
           if(!ask_hack())
               {
               free(rom);
               rom = NULL;
               free(ROM_HEADER);
               ROM_HEADER = NULL;
               return -3;
               }
           }
       if(s[i]=='b')
            {
            if(!ask_bad())
                {
                free(rom);
                rom = NULL;
                free(ROM_HEADER);
                ROM_HEADER = NULL;
                return -3;
                }
            }
        }
    strcpy(ROM_SETTINGS.goodname, entry->goodname);

    if(entry->savetype==EEPROM_16KB)
        { ROM_SETTINGS.eeprom_16kb = 1; }
    printf("EEPROM type: %d\n", ROM_SETTINGS.eeprom_16kb);
    return 0;
}

