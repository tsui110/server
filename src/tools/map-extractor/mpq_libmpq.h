/**
 * mangos-zero is a full featured server for World of Warcraft in its vanilla
 * version, supporting clients for patch 1.12.x.
 *
 * Copyright (C) 2005-2013  MaNGOS project <http://getmangos.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#ifndef MPQ_H
#define MPQ_H

#include <string.h>
#include <ctype.h>
#include <vector>
#include <iostream>
#include <deque>
#include <loadlib.h>
#include <libmpq/mpq.h>

using namespace std;

/**
 * @brief
 *
 */
class MPQArchive
{

    public:
        mpq_archive mpq_a; /**< TODO */

        /**
         * @brief
         *
         * @param filename
         */
        MPQArchive(const char* filename);
        /**
         * @brief
         *
         */
        void close();

        /**
         * @brief
         *
         * @param Input
         * @param Offset
         * @return uint32
         */
        uint32 HashString(const char* Input, uint32 Offset)
        {
            uint32 seed1 = 0x7fed7fed;
            uint32 seed2 = 0xeeeeeeee;

            for (uint32 i = 0; i < strlen(Input); i++)
            {
                uint32 val = toupper(Input[i]);
                seed1 = mpq_a.buf[Offset + val] ^(seed1 + seed2);
                seed2 = val + seed1 + seed2 + (seed2 << 5) + 3;
            }

            return seed1;
        }
        /**
         * @brief
         *
         * @param Filename
         * @return mpq_hash
         */
        mpq_hash GetHashEntry(const char* Filename)
        {
            uint32 index = HashString(Filename, 0);
            index &= mpq_a.header->hashtablesize - 1;
            uint32 name1 = HashString(Filename, 0x100);
            uint32 name2 = HashString(Filename, 0x200);

            for (uint32 i = index; i < mpq_a.header->hashtablesize; ++i)
            {
                mpq_hash hash = mpq_a.hashtable[i];
                if (hash.name1 == name1 && hash.name2 == name2) { return hash; }
            }

            mpq_hash nullhash;
            nullhash.blockindex = 0xFFFFFFFF;
            return nullhash;
        }

        /**
         * @brief
         *
         * @param filelist
         */
        void GetFileListTo(vector<string>& filelist)
        {
            mpq_hash hash = GetHashEntry("(listfile)");
            uint32 blockindex = hash.blockindex;

            if ((blockindex == 0xFFFFFFFF) || (blockindex == 0))
                { return; }

            uint32 size = libmpq_file_info(&mpq_a, LIBMPQ_FILE_UNCOMPRESSED_SIZE, blockindex);
            char* buffer = new char[size];

            libmpq_file_getdata(&mpq_a, hash, blockindex, (unsigned char*)buffer);

            char seps[] = "\n";
            char* token;

            token = strtok(buffer, seps);
            uint32 counter = 0;
            while ((token != NULL) && (counter < size))
            {
                //cout << token << endl;
                token[strlen(token) - 1] = 0;
                string s = token;
                filelist.push_back(s);
                counter += strlen(token) + 2;
                token = strtok(NULL, seps);
            }

            delete[] buffer;
        }
};
/**
 * @brief
 *
 */
typedef std::deque<MPQArchive*> ArchiveSet;

/**
 * @brief
 *
 */
class MPQFile
{
        //MPQHANDLE handle;
        bool eof; /**< TODO */
        char* buffer; /**< TODO */
        size_t pointer, size; /**< TODO */

        /**
         * @brief disable copying
         *
         * @param f
         */
        MPQFile(const MPQFile& f) {}
        /**
         * @brief
         *
         * @param f
         */
        void operator=(const MPQFile& f) {}

    public:
        /**
         * @brief
         *
         * @param filename filenames are not case sensitive
         */
        MPQFile(const char* filename);
        /**
         * @brief
         *
         */
        ~MPQFile() { close(); }
        /**
         * @brief
         *
         * @param dest
         * @param bytes
         * @return size_t
         */
        size_t read(void* dest, size_t bytes);
        /**
         * @brief
         *
         * @return size_t
         */
        size_t getSize() { return size; }
        /**
         * @brief
         *
         * @return size_t
         */
        size_t getPos() { return pointer; }
        /**
         * @brief
         *
         * @return char
         */
        char* getBuffer() { return buffer; }
        /**
         * @brief
         *
         * @return char
         */
        char* getPointer() { return buffer + pointer; }
        /**
         * @brief
         *
         * @return bool
         */
        bool isEof() { return eof; }
        /**
         * @brief
         *
         * @param offset
         */
        void seek(int offset);
        /**
         * @brief
         *
         * @param offset
         */
        void seekRelative(int offset);
        /**
         * @brief
         *
         */
        void close();
};

/**
 * @brief
 *
 * @param fcc
 */
inline void flipcc(char* fcc)
{
    char t;
    t = fcc[0];
    fcc[0] = fcc[3];
    fcc[3] = t;
    t = fcc[1];
    fcc[1] = fcc[2];
    fcc[2] = t;
}

#endif
