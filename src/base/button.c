﻿/**
 * File:   button.h
 * Author: AWTK Develop Team
 * Brief:  button
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-01-28 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/mem.h"
#include "base/timer.h"
#include "base/utils.h"
#include "base/button.h"

static ret_t button_on_paint_self(widget_t* widget, canvas_t* c) {
  return widget_paint_helper(widget, c, NULL, NULL);
}

static ret_t button_on_repeat(const timer_info_t* info) {
  pointer_event_t evt;
  button_t* button = BUTTON(info->ctx);
  widget_t* widget = WIDGET(info->ctx);

  evt.x = 0;
  evt.y = 0;
  evt.e = event_init(EVT_CLICK, widget);
  button->repeat_nr++;
  widget_dispatch(widget, (event_t*)&evt);

  if (button->repeat_nr == 4) {
    evt.e = event_init(EVT_LONG_PRESS, widget);
    widget_dispatch(widget, (event_t*)&evt);
  }

  return RET_REPEAT;
}

static ret_t button_on_event(widget_t* widget, event_t* e) {
  uint16_t type = e->type;
  button_t* button = BUTTON(widget);

  switch (type) {
    case EVT_POINTER_DOWN:
      button->repeat_nr = 0;
      widget_set_state(widget, WIDGET_STATE_PRESSED);
      if (button->timer_id != TK_INVALID_ID) {
        timer_remove(button->timer_id);
        button->timer_id = TK_INVALID_ID;
      }
      if (button->repeat > 0) {
        button->timer_id = timer_add(button_on_repeat, widget, button->repeat);
      }
      widget_grab(widget->parent, widget);
      break;
    case EVT_POINTER_UP: {
      pointer_event_t evt = *(pointer_event_t*)e;

      widget_ungrab(widget->parent, widget);
      evt.e = event_init(EVT_CLICK, widget);
      widget_set_state(widget, WIDGET_STATE_NORMAL);
      widget_dispatch(widget, (event_t*)&evt);
      button->repeat_nr = 0;
      if (button->timer_id != TK_INVALID_ID) {
        timer_remove(button->timer_id);
        button->timer_id = TK_INVALID_ID;
      }
      break;
    }
    case EVT_POINTER_LEAVE:
      widget_set_state(widget, WIDGET_STATE_NORMAL);
      break;
    case EVT_POINTER_ENTER:
      widget_set_state(widget, WIDGET_STATE_OVER);
      break;
    default:
      break;
  }

  return RET_OK;
}

ret_t button_set_repeat(widget_t* widget, int32_t repeat) {
  button_t* button = BUTTON(widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  button->repeat = repeat;

  return RET_OK;
}

static ret_t button_get_prop(widget_t* widget, const char* name, value_t* v) {
  button_t* button = BUTTON(widget);
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_REPEAT)) {
    value_set_int(v, button->repeat);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t button_set_prop(widget_t* widget, const char* name, const value_t* v) {
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_REPEAT)) {
    return button_set_repeat(widget, value_int(v));
  }

  return RET_NOT_FOUND;
}

static ret_t button_destroy(widget_t* widget) {
  button_t* button = BUTTON(widget);
  if (button->timer_id != TK_INVALID_ID) {
    timer_remove(button->timer_id);
    button->timer_id = TK_INVALID_ID;
  }

  return RET_OK;
}

static const widget_vtable_t s_button_vtable = {.type_name = WIDGET_TYPE_BUTTON,
                                                .on_event = button_on_event,
                                                .set_prop = button_set_prop,
                                                .get_prop = button_get_prop,
                                                .destroy = button_destroy,
                                                .on_paint_self = button_on_paint_self};

widget_t* button_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = NULL;
  button_t* button = TKMEM_ZALLOC(button_t);
  return_value_if_fail(button != NULL, NULL);

  widget = WIDGET(button);
  widget->vt = &s_button_vtable;
  widget_init(widget, parent, WIDGET_BUTTON);
  widget_move_resize(widget, x, y, w, h);
  button->timer_id = TK_INVALID_ID;

  widget_set_state(widget, WIDGET_STATE_NORMAL);

  return widget;
}
