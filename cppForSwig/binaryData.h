#ifndef _binaryData_H_
#define _binaryData_H_

#include <stdio.h>
#ifdef WIN32
   #include <cstdint>
#else
   #include <stdlib.h>
   #include <inttypes.h>   
   #include <cstring>
#endif
#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

#define DEFAULT_BUFFER_SIZE 25*1048576

using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class binaryData
{
public:


   /////////////////////////////////////////////////////////////////////////////
   binaryData(void) : data_(0), nBytes_(0)     {                         }
   binaryData(size_t sz)                       { alloc(sz);              }
   binaryData(uint8_t* inData, size_t sz)      { copyFrom(inData, sz);   }
   binaryData(uint8_t* dstart, uint8_t* dend ) { copyFrom(dstart, dend); }
   binaryData(string str)                      { copyFrom(str);          }
   binaryData(binaryData const & bd)           { copyFrom(bd);           }

   /////////////////////////////////////////////////////////////////////////////
   uint8_t * getPtr(void)                   { return &(data_[0]); }
   size_t getSize(void) const               { return nBytes_; }
   uint8_t const * getConstPtr(void) const  { return &(data_[0]); }

   /////////////////////////////////////////////////////////////////////////////
   // We allocate space as necesssary
   void copyFrom(uint8_t const * inData)                     { memcpy( &(data_[0]), inData, (size_t)nBytes_); }
   void copyFrom(uint8_t const * inData, size_t sz)          { if(sz!=nBytes_) alloc(sz); memcpy( &(data_[0]), inData, sz); }
   void copyFrom(uint8_t const * start, uint8_t const * end) { copyFrom( start, (end-start)); }  // [start, end)
   void copyFrom(string const & str)                         { copyFrom( (uint8_t*)str.c_str(), str.size()); } 
   void copyFrom(binaryData const & bd)                      { copyFrom( bd.getConstPtr(), bd.getSize() ); }

   /////////////////////////////////////////////////////////////////////////////
   // UNSAFE -- you don't know if outData holds enough space for this
   void copyTo(uint8_t* outData) const { memcpy( outData, &(data_[0]), (size_t)nBytes_); }
   void copyTo(uint8_t* outData, size_t sz) const { memcpy( outData, &(data_[0]), (size_t)sz); }
   void copyTo(uint8_t* outData, size_t offset, size_t sz) const { memcpy( outData, &(data_[offset]), (size_t)sz); }

   uint8_t & operator[](size_t i)       { return data_[i]; }
   uint8_t   operator[](size_t i) const { return data_[i]; } 

   /////////////////////////////////////////////////////////////////////////////
   bool operator<(binaryData const & bd2) const
   {
      int minLen = min(nBytes_, bd2.nBytes_);
      for(int i=0; i<minLen; i++)
      {
         if( data_[i] == bd2.data_[i] )
            continue;
         return data_[i] < bd2.data_[i];
      }
      return (nBytes_ < bd2.nBytes_);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool operator==(binaryData const & bd2) const
   {
      if(nBytes_ != bd2.nBytes_)
         return false;
      for(unsigned int i=0; i<nBytes_; i++)
         if( data_[i] != bd2.data_[i] )
            return false;
      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool operator>(binaryData const & bd2) const
   {
      int minLen = min(nBytes_, bd2.nBytes_);
      for(int i=0; i<minLen; i++)
      {
         if( data_[i] == bd2.data_[i] )
            continue;
         return data_[i] > bd2.data_[i];
      }
      return (nBytes_ > bd2.nBytes_);
   }

   /////////////////////////////////////////////////////////////////////////////
   // These are always memory-safe
   void copyTo(string & str) { str.assign( (char const *)(&(data_[0])), nBytes_); }
   string toString(void) const { return string((char const *)(&(data_[0])), nBytes_); }

   void resize(size_t sz) { data_.resize(sz); nBytes_ = sz;}

   /////////////////////////////////////////////////////////////////////////////
   // Swap endianness of the bytes in the index range [pos1, pos2)
   binaryData& swapEndian(size_t pos1=0, size_t pos2=0)
   {
      if(pos2 <= pos1)
         pos2 = nBytes_;

      size_t totalBytes = pos2-pos1;
      for(size_t i=0; i<(totalBytes/2); i++)
      {
         uint8_t d1    = data_[pos1+i];
         data_[pos1+i] = data_[pos2-(i+1)];
         data_[pos2-(i+1)] = d1;
      }
      return (*this);
   }

   /////////////////////////////////////////////////////////////////////////////
   string toHex(void)
   {
      static char hexLookupTable[16] = {'0','1','2','3',
                                        '4','5','6','7',
                                        '8','9','a','b',
                                        'c','d','e','f' };
      vector<int8_t> outStr(2*nBytes_);
      for( size_t i=0; i<nBytes_; i++)
      {
         uint8_t nextByte = data_[i];
         outStr[2*i  ] = hexLookupTable[ (nextByte >> 4) & 0x0F ];
         outStr[2*i+1] = hexLookupTable[ (nextByte     ) & 0x0F ];
      }
      return string((char const *)(&(outStr[0])), 2*nBytes_);
   }

   /////////////////////////////////////////////////////////////////////////////
   void createFromHex(string const & str)
   {
      static uint8_t binLookupTable[256] = { 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0a, 0x0b, 0x0c, 0x0d, 
            0x0e, 0x0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

      assert(str.size()%2 == 0);
      int newLen = str.size() / 2;
      alloc(newLen);

      for(int i=0; i<newLen; i++)
      {
         uint8_t char1 = binLookupTable[ (uint8_t)str[2*i  ] ];
         uint8_t char2 = binLookupTable[ (uint8_t)str[2*i+1] ];
         data_[i] = (char1 << 4) | char2;
      }
   }

private:
   vector<uint8_t> data_;
   size_t nBytes_;


private:
   void alloc(size_t sz)
   {
      data_ = vector<uint8_t>(sz);
      nBytes_ = sz;
   }

};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class binaryReader
{
public:
   /////////////////////////////////////////////////////////////////////////////
   binaryReader(int sz=0) :
      bdStr_(sz),
      totalSize_(sz),
      pos_(0)
   {
      // Nothing needed here
   }

   /////////////////////////////////////////////////////////////////////////////
   binaryReader(binaryData const & toRead) :
      bdStr_(toRead),
      totalSize_(toRead.getSize()),
      pos_(0)
   {
      // Nothing needed here
   }

   /////////////////////////////////////////////////////////////////////////////
   void advance(uint32_t nBytes) 
   { 
      pos_ += nBytes;  
      pos_ = min(pos_, totalSize_);
   }

   /////////////////////////////////////////////////////////////////////////////
   void rewind(uint32_t nBytes) 
   { 
      pos_ -= nBytes;  
      pos_ = max(pos_, (uint32_t)0);
   }

   /////////////////////////////////////////////////////////////////////////////
   void resize(uint32_t nBytes)
   {
      bdStr_.resize(nBytes);
      pos_ = min(nBytes, pos_);
   }

   /////////////////////////////////////////////////////////////////////////////
   uint64_t get_var_int(uint8_t* nRead=NULL)
   {
      uint8_t firstByte = bdStr_[pos_];

      if(firstByte < 0xfd)
      {
         if(nRead != NULL) *nRead = 1;
         pos_ += 1;
         return (uint64_t)firstByte;
      }
      if(firstByte == 0xfd)
      {
         if(nRead != NULL) *nRead = 3;
         pos_ += 3;
         return (uint64_t)(*(uint16_t*)(bdStr_.getPtr() + pos_ + 1));
         
      }
      else if(firstByte == 0xfe)
      {
         if(nRead != NULL) *nRead = 5;
         pos_ += 5;
         return (uint64_t)(*(uint32_t*)(bdStr_.getPtr() + pos_ + 1));
      }
      else //if(firstByte == 0xff)
      {
         if(nRead != NULL) *nRead = 9;
         pos_ += 9;
         return *(uint64_t*)(bdStr_.getPtr() + pos_ + 1);
      }
   }


   /////////////////////////////////////////////////////////////////////////////
   uint8_t get_uint8_t(void)
   {
      uint8_t outVal = bdStr_[pos_];
      pos_ += 1;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint16_t get_uint16_t(void)
   {
      uint16_t outVal = *(uint16_t*)(bdStr_.getPtr() + pos_);
      pos_ += 2;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint32_t get_uint32_t(void)
   {
      uint32_t outVal = *(uint32_t*)(bdStr_.getPtr() + pos_);
      pos_ += 4;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint64_t get_uint64_t(void)
   {
      uint64_t outVal = *(uint64_t*)(bdStr_.getPtr() + pos_);
      pos_ += 8;
      return outVal;
   }

   /////////////////////////////////////////////////////////////////////////////
   void get_binaryData(binaryData & bdTarget, uint32_t nBytes)
   {
      bdTarget.copyFrom( bdStr_.getPtr() + pos_, nBytes);
      pos_ += nBytes;
   }

   /////////////////////////////////////////////////////////////////////////////
   void get_binaryData(uint8_t* targPtr, uint32_t nBytes)
   {
      bdStr_.copyTo(targPtr, pos_, nBytes);
      pos_ += nBytes;
   }


   /////////////////////////////////////////////////////////////////////////////
   // Take the remaining buffer and shift it to the front
   // then return a pointer to where the old data ends
   //
   //                                      
   //  Before:                             pos
   //                                       |
   //                                       V
   //             [ a b c d e f g h i j k l m n o p q r s t]
   //
   //  After:      pos           return*
   //               |               |
   //               V               V
   //             [ m n o p q r s t - - - - - - - - - - - -]
   //                                 
   //
   pair<uint8_t*, int> rotateRemaining(void)
   {
      uint32_t nRemain = getSizeRemaining();
      //if(pos_ > nRemain+1)
         //memcpy(bdStr_.getPtr(), bdStr_.getPtr() + pos_, nRemain);
      //else
         memmove(bdStr_.getPtr(), bdStr_.getPtr() + pos_, nRemain);

      pos_ = 0;

      return make_pair(bdStr_.getPtr() + nRemain, totalSize_ - nRemain);
   }

   /////////////////////////////////////////////////////////////////////////////
   void     resetPosition(void)           { pos_ = 0; }
   uint32_t getPosition(void) const       { return pos_; }
   uint32_t getSize(void) const           { return totalSize_; }
   uint32_t getSizeRemaining(void) const  { return totalSize_ - pos_; }
   bool     isEndOfStream(void) const     { return pos_ >= totalSize_; }
   uint8_t* exposeDataPtr(void)           { return bdStr_.getPtr(); }

private:
   binaryData bdStr_;
   uint32_t totalSize_;
   uint32_t pos_;

};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class binaryStreamBuffer
{

public:

   /////////////////////////////////////////////////////////////////////////////
   binaryStreamBuffer(string filename="", uint32_t bufSize=DEFAULT_BUFFER_SIZE) :
      binReader_(bufSize),
      streamPtr_(NULL),
      weOwnTheStream_(false),
      bufferSize_(bufSize),
      fileBytesRemaining_(0)
   {
      if( filename.size() > 0 )
      {
         streamPtr_ = new ifstream;
         weOwnTheStream_ = true;
         ifstream* ifstreamPtr = static_cast<ifstream*>(streamPtr_);
         ifstreamPtr->open(filename.c_str(), ios::in | ios::binary);
         if( !ifstreamPtr->is_open() )
         {
            cerr << "Could not open file for reading!  File: " << filename.c_str() << endl;
            cerr << "Aborting!" << endl;
            assert(false);
         }

         ifstreamPtr->seekg(0, ios::end);
         totalStreamSize_  = (uint32_t)ifstreamPtr->tellg();
         fileBytesRemaining_ = totalStreamSize_;
         ifstreamPtr->seekg(0, ios::beg);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void attachAsStreamBuffer(istream & is, 
                             uint32_t streamSize,
                             uint32_t bufSz=DEFAULT_BUFFER_SIZE)
   {
      if(streamPtr_ != NULL && weOwnTheStream_)
      {
         static_cast<ifstream*>(streamPtr_)->close();
         delete streamPtr_;
      }

      streamPtr_           = &is;
      fileBytesRemaining_  = streamSize;
      totalStreamSize_     = streamSize;
      bufferSize_          = bufSz;
      binReader_.resize(bufferSize_);
   }


   
   /////////////////////////////////////////////////////////////////////////////
   // Refills the buffer from the stream, returns true if there is more data 
   // left in the stream
   bool streamPull(void)
   {
      uint32_t prevBufSizeRemain = binReader_.getSizeRemaining();
      if(fileBytesRemaining_ == 0)
         return false;

      if( binReader_.getPosition() <= 0)
      {
         // No data to shuffle, just pull from the stream buffer
         if(fileBytesRemaining_ > binReader_.getSize())
         {
            // Enough left in the stream to fill the entire buffer
            streamPtr_->read((char*)(binReader_.exposeDataPtr()), binReader_.getSize());
            fileBytesRemaining_ -= binReader_.getSize();
         }
         else
         {
            // The buffer is bigger than the remaining stream size
            streamPtr_->read((char*)(binReader_.exposeDataPtr()), fileBytesRemaining_);
            binReader_.resize(fileBytesRemaining_);
            fileBytesRemaining_ = 0;
         }
         
      }
      else
      {
         // The buffer needs to be refilled but has leftover data at the end
         pair<uint8_t*, int> leftover = binReader_.rotateRemaining();
         uint8_t* putNewDataPtr = leftover.first;
         uint32_t numBytes      = leftover.second;

         if(fileBytesRemaining_ > numBytes)
         {
            // Enough data left in the stream to fill the entire buffer
            streamPtr_->read((char*)putNewDataPtr, numBytes);
            fileBytesRemaining_ -= numBytes;
         }
         else
         {
            // The buffer is bigger than the remaining stream size
            streamPtr_->read((char*)putNewDataPtr, fileBytesRemaining_);
            binReader_.resize(fileBytesRemaining_+ prevBufSizeRemain); 
            fileBytesRemaining_ = 0;
         }
      }

      return true;
   }

   /////////////////////////////////////////////////////////////////////////////
   binaryReader& reader(void)
   {
      return binReader_;
   }

   /////////////////////////////////////////////////////////////////////////////
   uint32_t getFileByteLocation(void)
   {
      return totalStreamSize_ - (fileBytesRemaining_ + binReader_.getSizeRemaining());
   }

   uint32_t getBufferSizeRemaining(void) { return binReader_.getSizeRemaining(); }
   uint32_t getFileSizeRemaining(void)   { return fileBytesRemaining_; }
   uint32_t getBufferSize(void)          { return binReader_.getSize(); }

private:

   binaryReader binReader_;
   istream* streamPtr_;
   bool     weOwnTheStream_;
   uint32_t bufferSize_;
   uint32_t totalStreamSize_;
   uint32_t fileBytesRemaining_;

};


#endif
