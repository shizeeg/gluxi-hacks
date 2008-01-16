#include "image2ascii.h"

#include <QImage>
#include <QtDebug>

static int const RESIZED_WIDTH=200;
static const double BLACK_LEVEL=0.7;

Image2Ascii::Image2Ascii(const QImage& source)
{
	source_=source;
	black_="0";
	white_="1";
	width_=50;
}

Image2Ascii::~Image2Ascii()
{
}

QString Image2Ascii::ascii()
{
	QImage converted=source_.scaledToWidth(RESIZED_WIDTH).convertToFormat(QImage::Format_Mono, Qt::MonoOnly);
	int w=converted.width();
	int h=converted.height();
	
	qDebug() << "Converted image: " << w << "x" << h;

	if (width_<=0 || width_>200)
		return QString::null;
	int hdw=w/width_;
		
	int hdh=hdw*2;
	if (hdh==0)
			return QString::null;
	int numH=h/hdh;
	int blackLevel=(int)(((double)(hdh*hdw))*BLACK_LEVEL);

	QString buf;
	for (int idxY=0; idxY<numH; ++idxY)
	{
		for (int idxH=0; idxH<width_; ++idxH)
		{
			int x0=idxH*hdw;
			int y0=idxY*hdh;
			int dx=0;
			int numBlack=0;
			while (dx<hdw)
			{
				int x=x0+(dx++);
				int dy=0;
				while (dy<hdh)
				{
					int y=y0+(dy++);
					if (converted.pixelIndex(x, y)>0)
						++numBlack;
				}
			}
			if (numBlack>=blackLevel)
				buf+=black_;
			else
				buf+=white_;
		}
		buf+="\n";
	}
	return buf;
}
