/** @file */
#pragma once
#include "ext/clipp.h"
/** Main function wrapper. */
#define MAIN(APP) int main(int c, char **v) {            \
  Main::IApp *app = new APP; Main m(app); return m(c,v); \
}
/** Main program class. */
struct Main {
  /** Application interface. */
  struct IApp {
    /** Argument parsing function.
     *
     * With a reasonable default.
     * @param[in] argc      Number of command line arguments.
     * @param[in] argv      Command line argument strings. */
    virtual void Parse(int argc, char **argv);
    /** Main thread function. */
    virtual void Run() = 0;
    /** Main test set. */
    virtual void Test() = 0;
    virtual ~IApp() = default;
    clipp::group m_cli;
  };
  /** Constructor.
   * @param[in] app   Application class. */
  Main(IApp *app);
  /** Entry point
   *
   * @param[in] argc  Number of command line arguments.
   * @param[in] argv  Command line argument strings.
   * Try-catch, run the app. */
  int operator()(int argc, char **argv);
  IApp *m_app;
};

