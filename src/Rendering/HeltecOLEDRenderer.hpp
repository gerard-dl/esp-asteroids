#ifndef _heltec_hpp_
#define _heltec_hpp_

#include "Renderer.h"

class Font;

typedef void *TaskHandle_t;

class HeltecOLEDRenderer : public Renderer
{
private:
  TaskHandle_t _draw_task_handle;

public:
  HeltecOLEDRenderer(float world_size, Font *font);
  void start();
  friend void oled_draw_task(void *param);
};

#endif