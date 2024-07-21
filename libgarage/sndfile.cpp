#include "sndfile.h"
/**********************************/
/* Audio interface implementation */
/**********************************/
Audio::OutFile::OutFile(const std::string &path) {
  m_sfInfo.channels = 1;
  m_sfInfo.samplerate = 48000;
  m_sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
  m_file = sf_open(path.c_str(), SFM_WRITE, &m_sfInfo);
  // @todo[240612_112117] Error checks.
}
Audio::OutFile::~OutFile() { sf_close(m_file); }
void Audio::OutFile::Write(float *x, unsigned long len) {
  sf_writef_float(m_file, x, len);
}
void Audio::OutFile::Write(double *x, unsigned long len) {
  sf_writef_double(m_file, x, len);
}
Audio::InFile::InFile(const std::string &path) {
  m_fileName = path;
  m_sfInfo.format = 0;
  m_file = sf_open(m_fileName.c_str(), SFM_READ, &m_sfInfo);
  // @todo[240612_112117] Error checks.
}
Audio::InFile::~InFile() { sf_close(m_file); }
void Audio::InFile::Read(float* x, unsigned long len) {
  sf_readf_float(m_file, x, len);
}
void Audio::InFile::Read(double* x, unsigned long len) {
  sf_readf_double(m_file, x, len);
}
void Audio::InFile::Reset() {
  sf_close(m_file);
  m_file = sf_open(m_fileName.c_str(), SFM_READ, &m_sfInfo);
}
