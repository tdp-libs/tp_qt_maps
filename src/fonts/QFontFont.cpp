#include "tp_qt_maps/fonts/QFontFont.h"

#include "tp_utils/RefCount.h"
#include "tp_utils/TPPixel.h"

#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>

namespace tp_qt_maps
{

//##################################################################################################
struct QFontFont::Private
{
  TP_REF_COUNT_OBJECTS("tp_qt_maps::QFontFont::Private");
  TP_NONCOPYABLE(Private);

  const QFont font;
  const QFontMetrics fontMetrics;

  //################################################################################################
  Private(const QFont& font_):
    font(font_),
    fontMetrics(font)
  {

  }
};

//##################################################################################################
QFontFont::QFontFont(const QFont& font):
  d(new Private(font))
{

}

//##################################################################################################
QFontFont::~QFontFont()
{
  delete d;
}

//##################################################################################################
void QFontFont::prepareGlyph(char16_t character, const std::function<void(const tp_maps::Glyph&)>& addGlyph) const
{
  tp_maps::Glyph glyph;

  QChar ch = character;

  auto rect = d->fontMetrics.boundingRect(ch);

  glyph.leftBearing   = float( d->fontMetrics.leftBearing(ch));  //Negative for values to the left of 0
  glyph.rightBearing  = float(-d->fontMetrics.rightBearing(ch)); //Positive to the right of kerningWidth
  glyph.topBearing    = float(-rect.y());                        //Positive above 0
  glyph.bottomBearing = float(-(rect.height()+rect.y()));        //Positive above 0

#if QT_VERSION >= 0x050B00
  glyph.kerningWidth = float(d->fontMetrics.horizontalAdvance(ch));
#else
  glyph.kerningWidth = float(d->fontMetrics.width(ch));
#endif

  const int width  = rect.width()  + 2;
  const int height = rect.height() + 2;

  QPoint offset;
  offset.setX((-rect.x()) + 1);
  offset.setY((-rect.y()) + 1);

  QImage image(width, height, QImage::Format_ARGB32);
  image.fill(QColor(255, 255, 255, 0));
  {
    QPainter painter(&image);
    painter.setPen(QColor(255, 255, 255, 255));
    painter.setFont(d->font);
    painter.drawText(offset, QString(ch));
    painter.end();
  }

  std::vector<TPPixel> data;
  data.resize(size_t(width) * size_t(height));

  glyph.w = width;
  glyph.h = height;
  glyph.data = data.data();

  auto dst = glyph.data;
  for(int y=height-1; y>=0; y--)
  {
    auto src = image.scanLine(y);
    for(int w=0; w<width; w++, dst++, src+=4)
    {
      dst->r = src[2];
      dst->g = src[1];
      dst->b = src[0];
      dst->a = src[3];
    }
  }

  addGlyph(glyph);
}

//##################################################################################################
float QFontFont::lineHeight() const
{
  return d->fontMetrics.lineSpacing();
}

}
