/** @file */
#pragma once
#include <vector>
#include <string>
#include <sndfile.h>
/** Audio utilities. */
namespace Audio {
  /** Sound file abstraction. */
  struct File {
    SNDFILE *m_file;    /**< Audio file object reference. */
    SF_INFO m_sfInfo;   /**< Audio file information. */
    std::string m_fileName; /**< Audio file name. */
  };
  /** Output audio file object. */
  struct OutFile : File {
    OutFile(const std::string &path); /**< Open output file at path. */
    ~OutFile(); /**< Close output file. */
    void Write(float *x, unsigned long len);
    /**< Write samples to output file. */
    void Write(double *x, unsigned long len);
    /**< Write samples to output file. */
    /** Generic write. */
    template<typename T> void Write(std::vector<T>& buffer) {
      Write(buffer.data(), buffer.size());
    }
  };
  /** Input audio file object. */
  struct InFile : File {
    InFile(const std::string &path); /**< Open output file at path. */
    ~InFile(); /**< Close output file. */
    void Read(float *x, unsigned long len);
    /**< Write samples to output file. */
    void Read(double *x, unsigned long len);
    /**< Write samples to output file. */
    void Reset(); /**< Reopen the file to read from start. */
    /** Generic write */
    template<typename T> void Read(std::vector<T>& buffer) {
      Read(buffer.data(), buffer.size());
    }
  };
};

