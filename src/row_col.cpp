// (c) Copyright 2022 Aaron Kimball

#include "uiwidgets.h"

/////////////////////////////// Rows /////////////////////////////////////

Rows::Rows(uint16_t numRows): UIWidget(), _elements(NULL), _heights(NULL) {
  setNumRows(numRows);
}

Rows::~Rows() {
  delete [] _heights;
  delete [] _elements;
}

/**
 * Update the number of rows.
 *
 * If there are fewer rows in the new array, the top-most rows are preserved. You must free any
 * UIWidgets that no longer have a home before calling this method.
 *
 * If there are more rows in the new array, properties like setFixedHeight() are not applied to
 * new row elements; you must re-apply them.
 */
void Rows::setNumRows(uint16_t numRows) {
  uint16_t oldNumRows = _numRows;
  UIWidget **oldElements = _elements;
  int16_t *oldHeights = _heights;

  _numRows = numRows;
  if (_numRows < 1) {
    _numRows = 1;
  }

  _elements = new UIWidget*[_numRows]();
  _heights = new int16_t[_numRows]();

  if (oldElements != NULL) {
    // Transfer over the elements and sizes that carry into the newly sized container.
    for (uint16_t i = 0; i < min(oldNumRows, _numRows); i++) {
      _elements[i] = oldElements[i];
      _heights[i] = oldHeights[i];
    }

    delete [] oldElements;
    delete [] oldHeights;
  }

  cascadeBoundingBox();
}

void Rows::render(TFT_eSPI &lcd, uint32_t renderFlags) {
  drawBackground(lcd, renderFlags);
  drawBorder(lcd, renderFlags);

  // TODO(aaron): If widgets have FLEX sizing, dynamically adjust the bounding boxes of all
  // elements before rendering each element.

  if (isFocused(renderFlags)) {
    renderFlags |= RF_FOCUSED; // propagate our focused nature to any child element(s).
  }

  for (uint16_t i = 0; i < _numRows; i++) {
    UIWidget *widget = _elements[i];
    if (widget != NULL) {
      widget->render(lcd, renderFlags);
    }
  }
}

void Rows::cascadeBoundingBox() {
  // Get the inner bounding box.
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);

  // To calculate how much space is given to EQUAL rows, we first subtract
  // the heights of any explicitly-set rows; EQUAL rows then share all the
  // space that is not explicitly claimed equally.
  int16_t claimedHeight = 0;
  int16_t rowsWithClaimedHeight = 0;
  for (uint16_t i = 0; i < _numRows; i++) {
    if (_heights[i] >= 0) {
      claimedHeight += _heights[i];
      rowsWithClaimedHeight++;
    }
  }

  int16_t equalRowsAvailableHeight = max(0, childH - claimedHeight);
  int16_t equalRowsCount = _numRows - rowsWithClaimedHeight;
  // This gives us the actual height to apportion to a row configured to EQUAL sizing.
  const int16_t equalRowHeight = equalRowsAvailableHeight / max(equalRowsCount, 1);

  // Now calculate the actual heights/offsets of each row.
  for (uint16_t i = 0; i < _numRows; i++) {
    int16_t elemH = min(_heights[i], childH);
    if (_heights[i] == EQUAL) {
      elemH = equalRowHeight;
    }

    // TODO(aaron): Handle FLEX sizing.
    if (_elements[i] != NULL) {
      _elements[i]->setBoundingBox(childX, childY, childW, elemH);
    }

    childY += elemH; // Next row is lower down, by the height of this elem.
  }
}

/**
 * Sets the element at the specified offset. If this clobbers an existing UIWidget,
 * you must free it first.
 */
void Rows::setRow(uint16_t offset, UIWidget *widget, int16_t height) {
  if (offset >= _numRows) {
    return; // Invalid.
  }

  _elements[offset] = widget;
  _heights[offset] = height;
  cascadeBoundingBox();
}

void Rows::setFixedHeight(int16_t height) {
  for (uint16_t i = 0; i < _numRows; i++) {
    _heights[i] = height;
  }

  cascadeBoundingBox();
}

void Rows::setRowHeight(uint16_t offset, int16_t height) {
  if (offset >= _numRows) {
    return; // Invalid.
  }

  _heights[offset] = height;
  cascadeBoundingBox();
}

int16_t Rows::getRowHeight(uint16_t offset) {
  if (offset >= _numRows) {
    return ERROR_INVALID_ELEMENT; // Invalid.
  }

  return _heights[offset];
}

int16_t Rows::getContentWidth(TFT_eSPI &lcd) const {
  // The width required for a Rows is the max width required by any child.
  int16_t w = 0;
  for (int i = 0; i < _numRows; i++) {
    if (_elements[i] != NULL) {
      w = max(w, _elements[i]->getContentWidth(lcd));
    }
  }

  return addBorderWidth(w);
}

int16_t Rows::getContentHeight(TFT_eSPI &lcd) const {
  // The height required for a Rows is the sum of the row heights allocated to the children.
  // If any child is allocated EQUAL or FLEX height, return the entire height of this object.
  int16_t h = 0;
  for (int i = 0; i < _numRows; i++) {
    if (_heights[i] == EQUAL || _heights[i] == FLEX) {
      return _h; // We're going to expand to our entire allocated area.
    }
    h += _heights[i];
  }

  return addBorderHeight(h);
}

bool Rows::redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags) {
  if (NULL == widget) {
    return false;
  } else if (this == widget) {
    render(lcd, renderFlags);
    return true;
  } else if (containsWidget(widget)) {
    for (unsigned int i = 0; i < _numRows; i++) {
      if (_elements[i] != NULL && _elements[i]->containsWidget(widget)) {
        // Found the row containing the widget.
        drawBackgroundUnderWidget(widget, lcd, renderFlags);
        return _elements[i]->redrawChildWidget(widget, lcd, renderFlags);
      }
    }
  }

  return false;
}

/////////////////////////////// Cols /////////////////////////////////////
// TODO(aaron): This is unfortunately a near code-clone of Rows, but for the
// nomenclature used (row vs col; height vs width). This should all likely be
// refactored into a single Span element and then have Rows and Cols subclass it
// for direction.

Cols::Cols(uint16_t numCols): UIWidget(), _elements(NULL), _widths(NULL) {
  setNumCols(numCols);
}

Cols::~Cols() {
  delete [] _widths;
  delete [] _elements;
}

/**
 * Update the number of columns.
 *
 * If there are fewer cols in the new array, the left-most cols are preserved. You must free any
 * UIWidgets that no longer have a home before calling this method.
 *
 * If there are more cols in the new array, properties like setFixedWidth() are not applied to
 * new column elements; you must re-apply them.
 */
void Cols::setNumCols(uint16_t numCols) {
  uint16_t oldNumCols = _numCols;
  UIWidget **oldElements = _elements;
  int16_t *oldWidths = _widths;

  _numCols = numCols;
  if (_numCols < 1) {
    _numCols = 1;
  }

  _elements = new UIWidget*[_numCols]();
  _widths = new int16_t[_numCols]();

  if (oldElements != NULL) {
    // Transfer over the elements and sizes that carry into the newly sized container.
    for (uint16_t i = 0; i < min(oldNumCols, _numCols); i++) {
      _elements[i] = oldElements[i];
      _widths[i] = oldWidths[i];
    }

    delete [] oldElements;
    delete [] oldWidths;
  }

  cascadeBoundingBox();
}

void Cols::render(TFT_eSPI &lcd, uint32_t renderFlags) {
  drawBackground(lcd, renderFlags);
  drawBorder(lcd, renderFlags);

  // TODO(aaron): If widgets have FLEX sizing, dynamically adjust the bounding boxes of all
  // elements before rendering each element.

  if (isFocused(renderFlags)) {
    renderFlags |= RF_FOCUSED; // propagate our focused nature to any child element(s).
  }

  for (uint16_t i = 0; i < _numCols; i++) {
    UIWidget *widget = _elements[i];
    if (widget != NULL) {
      widget->render(lcd, renderFlags);
    }
  }
}

void Cols::cascadeBoundingBox() {
  // Get the inner bounding box.
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);

  // To calculate how much space is given to EQUAL columns, we first subtract
  // the widths of any explicitly-set columns; EQUAL columns then share all the
  // space that is not explicitly claimed equally.
  int16_t claimedWidth = 0;
  int16_t colsWithClaimedWidth = 0;
  for (uint16_t i = 0; i < _numCols; i++) {
    if (_widths[i] >= 0) {
      claimedWidth += _widths[i];
      colsWithClaimedWidth++;
    }
  }
  int16_t equalColsAvailableWidth = max(0, childW - claimedWidth);
  int16_t equalColsCount = _numCols - colsWithClaimedWidth;
  // This gives us the actual width to apportion to a col configured to EQUAL sizing.
  const int16_t equalColWidth = equalColsAvailableWidth / max(equalColsCount, 1);

  // Now calculate the actual widths/offsets of each col.
  for (uint16_t i = 0; i < _numCols; i++) {
    int16_t elemW = min(_widths[i], childW);
    if (_widths[i] == EQUAL) {
      elemW = equalColWidth;
    }

    // TODO(aaron): Handle FLEX sizing.
    if (_elements[i] != NULL) {
      _elements[i]->setBoundingBox(childX, childY, elemW, childH);
    }

    childX += elemW; // Next col is further to the right, by the width of this elem.
  }
}

/**
 * Sets the element at the specified offset. If this clobbers an existing UIWidget,
 * you must free it first.
 */
void Cols::setColumn(uint16_t offset, UIWidget *widget, int16_t width) {
  if (offset >= _numCols) {
    return; // Invalid.
  }

  _elements[offset] = widget;
  _widths[offset] = width;
  cascadeBoundingBox();
}

void Cols::setFixedWidth(int16_t width) {
  for (uint16_t i = 0; i < _numCols; i++) {
    _widths[i] = width;
  }

  cascadeBoundingBox();
}

void Cols::setColumnWidth(uint16_t offset, int16_t width) {
  if (offset >= _numCols) {
    return; // Invalid.
  }

  _widths[offset] = width;
  cascadeBoundingBox();
}

int16_t Cols::getColumnWidth(uint16_t offset) {
  if (offset >= _numCols) {
    return ERROR_INVALID_ELEMENT; // Invalid.
  }

  return _widths[offset];
}

int16_t Cols::getContentHeight(TFT_eSPI &lcd) const {
  // The height required for a Cols is the max height required by any child.
  int16_t h = 0;
  for (int i = 0; i < _numCols; i++) {
    if (_elements[i] != NULL) {
      h = max(h, _elements[i]->getContentHeight(lcd));
    }
  }

  return addBorderHeight(h);
}

int16_t Cols::getContentWidth(TFT_eSPI &lcd) const {
  // The width required for a Cols is the sum of the col widths allocated to the children.
  // If any child is allocated EQUAL or FLEX height, return the entire width of this object.
  int16_t w = 0;
  for (int i = 0; i < _numCols; i++) {
    if (_widths[i] == EQUAL || _widths[i] == FLEX) {
      return _w; // We're going to expand to our entire allocated area.
    }
    w += _widths[i];
  }

  return addBorderWidth(w);
}

bool Cols::redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags) {
  if (NULL == widget) {
    return false;
  } else if (this == widget) {
    render(lcd, renderFlags);
    return true;
  } else if (containsWidget(widget)) {
    for (unsigned int i = 0; i < _numCols; i++) {
      if (_elements[i] != NULL && _elements[i]->containsWidget(widget)) {
        // Found the column containing the widget.
        drawBackgroundUnderWidget(widget, lcd, renderFlags);
        return _elements[i]->redrawChildWidget(widget, lcd, renderFlags);
      }
    }
  }

  return false;
}
