#include <exception>
#include <iostream>
#include <cstdlib>
#include "ext/clipp.h"
#include "main.h"
using namespace std;
Main::Main(IApp *app) : m_app(app) {}
int Main::operator()(int argc, char **argv){
  try { m_app->Parse(argc, argv); m_app->Run(); }
  catch (const exception& e) { cerr << e.what() << endl; return EXIT_FAILURE; }
  return EXIT_SUCCESS;
}
void Main::IApp::Parse(int argc, char **argv) {
  using namespace clipp;
  if (!parse(argc, argv, m_cli)) {
    cout << make_man_page(m_cli, argv[0]);
    throw runtime_error("Failed parsing command line arguments!");
  }
}

