

uiwidgets -- Mini UI widget library
===================================

A UI widget library for drawing on an LCD in Arduino. This library uses the `TFT_eSPI` library for
rendering to a screen; however, it provides a UI-widget-centric way of describing content rather
than requiring that you performed fine-grained rendering direction, and assists with automatic
layout of content elements on the screen.

This depends on the `Seeed_Arduino_LCD` or `TFT_eSPI` library for the underlying
display control.

* https://github.com/Seeed-Studio/Seeed_Arduino_LCD/
* https://github.com/Bodmer/TFT_eSPI

Compiling
=========

Add the `UIWidgets` library to your Arduino sketch in the IDE.

Or, if you are compiling on the command line:

```
make all install
```

Using `make` depends on my Arduino Makefile (https://github.com/kimballa/arduino-makefile).

If building with a Makefile, you must also download, compile, and `make install` any dependency
libraries. See the `libs` line in the `Makefile` for a complete list of dependencies.

Layout System
=============

The layout of widgets is based on a nesting system. The top-most visible object is always a
`Screen`. You can have multiple `Screens`, but only one will show at a time. The Screen can hold one
element, which may be something that visibly displays, like a `StrLabel`, or it could be a `Rows`
or `Cols`; these can be configured to have N items down or across (which may be more `Rows` or
`Cols`). Each object has an x, y, width, and height specified in pixels.  As containing objects are
moved by layout changes, they cascade the position and size offsets to their children.

In general, widgets expand to fill the space available to them. Laying out multiple widgets can be
accomplished by nesting `Rows` and `Cols` objects to make a grid, where each row or column therein
can have a child widget (which may itself be `Rows` or `Cols`) with user-controlled height or width.
The height/width can also be specified as `EQUAL`; any such `EQUAL` rows or columns share equally
among all the vertical or horizontal space available. i.e., in a 320x240 display, there are 240
vertical pixels available to a top-level `Rows` object. If one row is specified to have a height of
`100`, and two rows are specified as `EQUAL`, the latter two columns will each have a height of 70
pixels ((240 - 100) / 2).

> **Tip:** Adding a single `EQUAL` row or column with a `NULL` UIWidget pointer in the middle of a `Rows`
> or `Cols` will make the previous rows/columns be top/left-justified and subsequent rows/columns be
> bottom/right-justified.

A `FLEX` layout option that depends on minimum size to present the specified content is planned but
not functional yet.

Each UIWidget knows its absolute coordinates, so if a small element of the screen is invalidated
due to changed data, a hierarchical sub-portion of the screen can be redrawn.

Widgets
=======

Screen
------
The top-most container; exactly one Screen can be shown at a time. No porcelain by default.
A `Screen` holds exactly one child `UIWidget`. The `Screen` itself is not technically a `UIWidget`;
it manages the overall connection to the screen hardware and redrawing of the screen.

Relevant methods:
* `void setWidget(UIWidget *w)`: Binds the top-level widget to the screen.
* `void render(uint32_t renderFlags = RF_NONE)`: Redraws the entire screen.
* `void renderWidget(UIWidget *w, uint32_t renderFlags = RF_NONE)`: Redraws only the part of the
  screen directly under the bounding box of `w`; `w` is usually nested inside the top-level widget,
  rather than indicating the top-level widget itself.
* `getWidth()`, `getHeight()`: Return the size of the screen
* `void setBackground(uint16_t backgroundColor`: Specify a background color to apply to the entire
  screen.

UIWidget
--------
Base class for all other classes in this library. Manages common capabilities associated
with widgets. For example, all widgets have the following methods you can call:

* `setBorder(border_flags_t f, uint16_t color)`: Sets whether one or more edges of the widget have a
  rectilinear border (or a rounded-rect border for the entire widget).
* `setBackground(uint16_t color)`: Sets the widget background color.
* `setFocus(bool focus)`, `isFocused()`: Controls whether this is the _focused_ widget. A widget
  with focus is rendered differently (usually inverted colors) than the other widgets.
* `setPadding(int16_t padL, int16_t padR, int16_t padTop, int16_t padBottom)`: Sets interior margin
  padding, in pixels, between the edge of the widget's rect and its inner content area.
  User-specified padding is additive to padding automatically added in the presence of a border
  created by `setBorder()`. Your padding parameters _may_ be negative.
* `getX()`, `getY()`, `getWidth()`, `getHeight()`: Each describes one parameter of the widget
  bounding box.
* `void getRect(int16_t &cx, int16_t &cy, int16_t &cw, int16_t ch)`: Get all bounding box
  parameters.
* `bool containsWidget(UIWidget *w)`: Return true if w is nested inside this widget.

### Non-interface methods

This object also has a `render()` method (and `redrawChildWidget()`), but rendering should actually
be drawn through the `Screen` rather than invoking this yourself; the `Screen` ensures that any
parent widgets have the opportunity to draw background coloring, which may show through if this
widget does not have a background color, or otherwise has elements showing the `TFT_TRANSPARENT`
color.

Similarly, the bounding box can be directly set with `setBoundingBox()`, but when many aspects of a
`UIWidget`'s state are changed, they automatically call `setBoundingBox()` on their child widgets,
which would override your preferences.

> **Important!** A widget only has a single bounding box of coordinates, and the parent widget is
> generally empowered to change those to reflect other necessary layout updates (presence of
> additional child widgets, or new context from its own parent). For this reason, each widget must
> have **exactly one** parent widget.
>
> For example, while you may need multiple labels that say
> `"Name"` on the display at once, you cannot repurpose a single `StrLabel` in multiple rows of a
> `Rows`. You must use a different `StrLabel` in each location (although they may share a common
> `const char *` array containing the actual text).
>
> Similarly, each `UIWidget` instance can only be used on a single `Screen`; do not attach UIWidgets
> to each other or to screens in such a way that any `UIWidget` has more than one parent `Screen` or
> `UIWidget`.

### Render performance tips and control

Drawing background rectangles of solid color is a very time-intensive activity and excessive
background drawing can cause noticible flickering of the display. You should call `setBackground()`
on as few elements as possible, to achieve as close to a one-and-done coloring of all pixels as
possible. Other widgets should be left at the default (`BG_NONE` aka `TRANSPARENT_COLOR` aka
`TFT_TRANSPARENT`), which suppresses background drawing.

If you need only a portion of the screen updated, calling `Screen.renderWidget()` to draw only the
invalidated subcomponent (and its children) is much faster than `render()`. You can further increase
speed by passing render flags that control its behavior:

* `RF_NONE`: No special handling
* `RF_NO_BACKGROUNDS`: Do not draw background flood fill rectangles.
* `RF_FOCUSED`: The widget in question is drawn as-if focused, regardless of its `setFocus()` bit.

In addition, the `VScroll` object takes additional render flags.


Panel
-----
A container for another item. This has no porcelain by default. A panel can have background color or
borders enabled in the usual way.

* `void setChild(UIWidget *widget)`: Sets the child widget of this panel.

Rows
----
A collection of N objects that will be displayed stacked on top of one another. Each row expands to
the full width of its container. The `Rows` can have a fixed per-row height, or each row can use
`EQUAL`; available space is shared equally among all rows specified with `EQUAL` height. There is
also a `FLEX` parameter to make row height content-sensitive, but this is not yet operational.

* `void setNumRows(uint16_t numRows)`: Specify the number of rows in the Rows object.
* `void setRow(uint16_t rowNum, UIWidget *widget, int16_t height)`: Specify the widget that goes in
  row `rowNum` (0 is the first/top row, `numRows-1` is the bottom/last row) as well as its height.
  Blank rows may be specified with `widget=NULL`.
* `setRowHeight(uint16_t rowNum, int16_t height)`: Control the height of a given row.
* `setFixedHeight(int16_t height)`: Apply a particular height value to all existing rows. Further
  row updates with `setRow()` will override this.


Cols
----
A collection of N objects that will be displayed next to one another, sequenced left-to-right. Each
column expands to the full height of its container. The `Cols` can have a fixed per-col width, or
each col can use `EQUAL`; available space is shared equally among all columns specified with `EQUAL`
width. There is also a `FLEX` parameter to make column width content-sensitive, but this is not yet
operational.

* `void setNumCols(uint16_t numCols)`: Specify the number of columns in the Cols object.
* `void setColumn(uint16_t colNum, UIWidget *widget, int16_t width)`: Specify the widget that goes in
  column `colNum` (0 is the first/left col, `numCols-1` is the right/last col) as well as its width.
  Blank columns may be specified with `widget=NULL`.
* `setColumnWidth(uint16_t rowNum, int16_t height)`: Control the width of a given column.
* `setFixedWidth(int16_t height)`: Apply a particular width value to all existing cols. Further
  column updates with `setColumn()` will override this.

Button
------
A selectable button with text in it.

* `void setText(const char *str)`: Sets the text to display in the Button to be backed by `str`. The
  lifetime of `str` must not end before the `Button` itself goes out of scope; Button does not make
  a copy of this string.

> **Tip:** You can render a button being "clicked" with `myScreen.renderWidget(&myButton,
> RF_FOCUSED);`

Label
--------
Displays some text. All label subclasses share common methods to control style:

* `void setFont(int fontId)`: Specify the id of a font within the font library built into `TFT_eSPI`
  to use for text rendering. Note that `TFT_eSPI` uses conditional compilation and `#define` flags
  to include or exclude different fonts, so this must match the number for a font that you have
  included in your build. Note also that this does not yet support the "FreeFont" fonts, only the
  numbered ones.
* `void setColor(uint16_t color)`: Set the foreground text color.

StrLabel
--------
Subclass of `Label`. Displays a string.

* `void setText(const char *str)`: Sets the text to display in the Button to be backed by `str`. The
  lifetime of `str` must not end before the `StrLabel` itself goes out of scope; StrLabel does not make
  a copy of this string.
* `void setText(const String &str)`: Sets the text to display in the Button to be backed by `str`. The
  lifetime of `str` must not end before the `StrLabel` itself goes out of scope; StrLabel does not make
  a copy of this string.

IntLabel
--------
Subclass of `Label`. Displays an integer.

* `void setValue(long val)`: Specifies the number to print to the screen.

FloatLabel
--------
Subclass of `Label`. Displays a float.

* `void setValue(float val)`: Specifies the number to print to the screen.
* `void setMaxDecimalDigits(uint8_t digits)`: Specifies the maximum number of digits to the right of
  the `.`  to render. `TFT_eSPI` limits this to at most 7.

VScroll
-------
A container that holds a variable number of items, more than can be shown on the screen.
A vertically-oriented scrollbar is on the right side; this can be scrolled to carousel which
elements are rendered to the screen. A line of this scrollable collection can be granted focus
by virtue of being the _selected_ element of the VScroll; the selection can be moved up and down, as
can the window of displayed elements.

Useful methods:

Managing content:
* `void add(UIWidget *widget)`: Add another entry to the bottom of the VScroll.
* `void remove(UIWidget *widget)`: Remove the specified child widget.
* `void clear()`: Remove all children

Content quantification:
* `size_t size()`: Number of entries
* `size_t position()`: The index of the entry at the top of the visible content window.
* `size_t bottomIdx()`: The index of the entry one past the bottom of the visible content window.

Managing the selection:
* `void setSelection(size_t idx)`: Set the index of the selected widget.
* `selectUp()`: Select the previous element.
* `selectDown()`: Select the next element.
* `size_t selectIdx()`: The index of the selected widget.
* `UIWidget *getSelected()`: Get the selected widget itself.

* `setItemHeight(int16_t newHeight)`: Set the row height available to each child widget to render
  in. Unlike `Rows`, the item height for VScroll children is set once and consistently applied to
  all child widgets including those added after the call to `setItemHeight()`.

* `void setContentBackground(uitn16_t color)`: Set the background color for the content area.
* `void setContentBackground(uitn16_t color)`: Set the background color for the scrollbar area.
* Note that `VScroll` does **not** respect the general `UIWidget::setBackground()` method.

Fine-grained render/animation control:
* `void renderScrollUp(TFT_eSPI &lcd, bool btnActive, uint32_t renderFlags=0)`: Render just the
  "scroll-up" caret. If the caret is being "pressed", `btnActive` should be `true`.
* `void renderScrollDown(TFT_eSPI &lcd, bool btnActive, uint32_t renderFlags=0)`: Render just the
  "scroll-down" caret. If the caret is being "pressed", `btnActive` should be `true`.
* The scrollbar itself and main content can be rendered in fine-grained fashion through
  `Screen::renderWidget()` with appropriate `renderFlags` bits set.

VScroll supports additional render flags when redrawing:

* `RF_VSCROLL_CONTENT`: Redraw the content area; do not also redraw the scrollbar unless explicitly
  asked to. (You may also add `RF_VSCROLL_SCROLLBAR`)
* `RF_VSCROLL_SCROLLBAR`: Redraw just the scrollbar (and its caret buttons); do not also redraw the
  content area unless explicitly asked to. (You may also add `RF_VSCROLL_CONTENT`)
* `RF_VSCROLL_SELECTED`: Only redraw the _selected_ entry of the VScroll's content area, as well as
  the most-previously _selected_ entry before this one.


Menu (TODO)
----
_Not yet implemented._

A collection of text lines that can each be selected.
(TODO: Does this take over the whole screen?)

Implementing your own widgets
=============================

If you want to create more widgets unique to your project, you can create subclasses of `UIWidget`.

These must implement the following methods:

* `virtual void render(TFT_eSPI &lcd, uint32_t renderFlags)` - Render the widget itself using the
  drawing methods of `lcd`. You must stick to the area of the screen specified by `UIWidget::getRect()`.
* `virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0)` - If
  `widget == this`, act like `render()`; otherwise, re-render any of your own content that falls within the
  bounding box of `widget`. If you have child widgets, you should test if
  `someChild->containsWidget(widget)` and if true for each/any of them, call `redrawChildWidget()`
  recursively.
* `virtual void cascadeBoundingBox()` - After your own bounding box is updated by your parent
  widget, the parent will invoke `childWidget.cascadeBoundingBox()` to notify you of the changed
  bounding box. Within this method, you should then call `setBoundingBox(x, y, w, h)` as appropriate
  on each of your own child widgets.
* `virtual int16_t getContentWidth(TFT_eSPI &lcd) const` - Return the minimum width required to draw
  your content plus any border/padding.
* `virtual int16_t getContentHeight(TFT_eSPI &lcd) const` - Return the minimum height required to
  draw your content plus any border/padding.

Helpful inherited methods
-------------------------

`UIWidget` contanis many helper methods likely useful to you:

* `void drawBackground(TFT_eSPI &lcd, uint32_t renderFlags)`: Draw the background color of your
  widget, if any. Typically called at the start of `render()`.
* `void drawBorder(TFT_eSPI &lcd, uint32_t renderFlags)`: Draw the border of your widget, if any.
  Typically called just after `drawBackground()` at the start of `render()`.
* `drawBackgroundUnderWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0)`: If your
  widget has a background color set, draw the background color under the rectangle specified by
  `widget`'s bounding box. Typically used within `redrawChildWidget()` instead of
  `drawBackground()`, as there is substantial performance savings to redrawing only the minimal part
  of the screen required versus redrawing the whole thing.
* `void getChildAreaBoundingBox(int16_t &childX, int16_t &childY, int16_t &childW, int16_t
  &childH)`: Returns the bounding box (in pixels) of the area where you should draw your content.
  This may be smaller than `getRect()`, as it accounts for space taken up by a border (if set) and
  user-specified padding from `setPadding()`.
* `int16_t addBorderWidth(int16_t contentWidth)`: Returns `contentWidth` plus any pixel width
  required to render borders or account for user-specified padding. Used in `getContentWidth()`.
* `int16_t addBorderHeight(int16_t contentHeight)`: Returns `contentHeight` plus any pixel height
  required to render borders or account for user-specified padding. Used in `getContentHeight()`.

License
=======

This project is licensed under the BSD 3-Clause license. See LICENSE.txt for complete details.
