/** @file */
#pragma once
#include <vector>
#include <optional>
/** Modular event generator, the namespace. */
namespace Meg {
  template<class T> using Maybe = std::optional<T>;
  using Count = unsigned long;
  /** Generic event format. */
  struct Gef { Maybe<Count> time; void* event; };
  /** Base module interface. */
  struct Module {
    virtual Gef* Process(Gef* input);
  };
}
