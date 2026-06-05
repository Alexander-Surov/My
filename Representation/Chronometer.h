#ifndef CHRONOMETER
#define CHRONOMETER

#include <chrono>
#include <iostream>
#include <iomanip>

namespace my {

  /**
   * @brief   A convenient tool for measuring excecution time of some process.
   *
   * @details Constructor:  ("...") -> sets a name for chronometer
   *
   *          Functions:    now    - print the chronometer's lifetime
   *                        stop   - stop time counting
   *                        resume - resume time counting
   *                        reset  - time is counted from now
   *                        on     - enables
   *                        off    - disables
   *
   *          When a chronometer is destructed its lifetime is printed.
   *
   * @note    Any manipulation with chronometers also consumes some time (~100 ms)
   */
  class Chronometer {
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::high_resolution_clock::time_point;
    using duration = std::chrono::duration<float>;


   private:
    const char* name_;

    time_point time_;
    time_point buffer_;

    bool is_stopped_;
    bool is_on_;


   public:
    explicit Chronometer(const char*&& name = "")
      : name_(name), time_(clock::now()), buffer_(), is_stopped_(false), is_on_(true) {
    }

    ~Chronometer() {
      now();
    }


   public:
    void now() const {
      if (is_on_)
        std::cout << std::fixed << std::setprecision(6)
                  << "\n-------------------------------\n"
                  << name_ << " - " << static_cast<duration>(is_stopped_ ? (buffer_ - time_) : (clock::now() - time_)).count() << " s"
                  << "\n-------------------------------\n";
    }
    void stop() {
      if (!is_stopped_ && is_on_) {
        is_stopped_ = true;
        buffer_ = clock::now();
      }
    }
    void resume() {
      if (is_stopped_ && is_on_) {
        is_stopped_ = false;
        time_ += (clock::now() - buffer_);
      }
    }
    void reset() {
      if (is_on_) {
        time_ = clock::now();
        is_stopped_ = false;
      }
    }

    void on() {
      is_on_ = true;

      if (is_stopped_) {
        is_stopped_ = false;
        time_ += (clock::now() - buffer_);
      }
    }
    void off() {
      is_on_ = false;

      if (!is_stopped_) {
        is_stopped_ = true;
        buffer_ = clock::now();
      }
    }
  };

}

#endif