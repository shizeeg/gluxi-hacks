#ifndef IMAGE2ASCII_H_
#define IMAGE2ASCII_H_

#include <QImage>
#include <QString>

class Image2Ascii
{
public:
	Image2Ascii(const QImage& source);
	virtual ~Image2Ascii();
	QString ascii();
	int width() const { return width_; }
	QString white() const { return white_; }
	QString black() const { return black_; }
	int setWidth(int width) { width_=width; }
	void setWhite(const QString& white) { white_=white; }
	void setBlack(const QString& black) { black_=black; }
private:
	QImage source_;
	int width_;
	QString white_;
	QString black_;
};

#endif /*IMAGE2ASCII_H_*/
