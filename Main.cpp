#include <bcm2835.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "LiquidCrystal.h"
#include "Ads1115.h"
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio/videoio.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp> //cvtColor
#include <opencv2/objdetect/objdetect.hpp> //CascadeClassifier
//#include <iostream>
#include <vector>
#define SANIYE (60)
#define BUZPIN (6)
using namespace cv;
using namespace std;
int main()
{
 if (!bcm2835_init())
 {
 printf("bcm2835_init failed. Are you running as root?\n");
 return 1;
 }

 if (!bcm2835_i2c_begin())
 {
 printf("bcm2835_i2c_begin failed. Are you running as root?\n");
 return 1;
 }

 bcm2835_gpio_fsel(BUZPIN, BCM2835_GPIO_FSEL_OUTP);
 bcm2835_gpio_write(BUZPIN, LOW);

 // LCD ekran kuruluyor
 LiquidCrystal* lcd = new LiquidCrystal(13,19,26,16,20,21);

 // Kameradan ateş algılamalarını tutacak olan kare nesneleri oluşturuluyor
 vector<Rect> detections;

 // Ateşi algılayan algoritma/tarama dosyası
 CascadeClassifier fire_cascade;
 if(fire_cascade.load("/home/pi/fire_detection.xml"))
 {
 //Kamera sınıfı oluşturuluyor
 VideoCapture camera(0);

 Mat gray, save_img, saturated; //frame'ler

 //Kamera vermı kontrol ediliyor.
 if(!camera.isOpened()) //yoksa bilgi lcd ekrana yazdırılıyor
 {
 lcd->clear();
 lcd->setCursor(0,0);
 lcd->sendText("Kamera bulunamadı");

 return 1;
 }

 bool isSmoke = false, isFire = false;
 //Sensörü okuyan analog entegrenin sınıfı
 Ads1115* ads = new Ads1115();

 lcd->clear();
 lcd->setCursor(0,0);
 lcd->sendText("Uygulama");
 lcd->setCursor(0,1);
 lcd->sendText("başlatılıyor...");

 bcm2835_delay(3000);

 uint8_t sayac = 0;
 uint16_t min = 65535;

 lcd->clear();
 lcd->setCursor(0,0);
 lcd->sendText("Sensor kalibre");

 // Sensör kalibre ediliyor
 while(sayac < SANIYE)
 {
 uint16_t sensor = ads->readADC_SingleEnded(0);
 min = min > sensor ? sensor : min;
 sayac++;
 lcd->setCursor(0,1);
 lcd->sendText(" ");
 lcd->setCursor(0,1);
 lcd->sendText("ediliyor " + to_string(SANIYE-sayac) + " sn");
 bcm2835_delay(1000);
 }

 lcd->clear();

 //printf("%u\n",min);

 stringstream ss;


 //Ana döngü
 while(1)
 {
 ///////////////////////////////////////////////////////////////
 //
 // Duman seviyesi ölçülüyor
 //
 ///////////////////////////////////////////////////////////////
 ss.str(string()); //ss boşaltılıyor
 lcd->setCursor(0,1); // 2. satır
 uint16_t sensor = ads->readADC_SingleEnded(0) - min;
 sensor = sensor > 32768 ? 65536 - sensor : sensor;
 //printf("%u\n",sensor);
 ss << "Duman: " << setw(5) << setfill(' ') << sensor << " ";
 lcd->sendText(ss.str());

 if(sensor > 6000)
 {
 isSmoke = true;
 //bcm2835_gpio_write(BUZPIN, HIGH);
 //bcm2835_delay(3000);
 }

 //bcm2835_gpio_write(BUZPIN, LOW);

 ///////////////////////////////////////////////////////////////
 //
 // Kamera ile ateş algılaması yapılıyor
 //
 ///////////////////////////////////////////////////////////////
 camera >> save_img; //kameradan 1 frame alınıyor
 save_img.convertTo(saturated, CV_8UC1, 1, -30); // renkler keskinleştiriliyor
 //alınan görüntü gri tonlu resme dönüştürülüyor
 cvtColor(saturated, gray, COLOR_BGR2GRAY); 
 

 // Ateş tespiti yapılıyor
 fire_cascade.detectMultiScale(gray, detections, 2,5);

 ss.str(string()); //ss boşaltılıyor
 lcd->setCursor(0,0); // 1. satır
 ss << "Ateş: " << setw(3) << setfill(' ') << detections.size() << " ";
 lcd->sendText(ss.str());

 if (detections.size() > 0) //ateş algılandı ise
 {
 //ateşin bulunduğu fotoğraf kaydediliyor
 imwrite("saturated.jpg", saturated);
 imwrite("normal.jpg", save_img);
 // alarmmmmm!!!!!
 isFire = true;
 }
// Büyük yangın, hem duman var, hem ateş
 if(isFire && isSmoke)
 {
 uint8_t i = 10;
 while(i-->0)
 {
 bcm2835_gpio_write(BUZPIN, HIGH);
 bcm2835_delay(200);
 bcm2835_gpio_write(BUZPIN, LOW);
 bcm2835_delay(200);
 }
 }
 else if (!isFire && isSmoke)
 {
 uint8_t i = 4;
 while(i-->0)
 {
 bcm2835_gpio_write(BUZPIN, HIGH);
 bcm2835_delay(500);
 bcm2835_gpio_write(BUZPIN, LOW);
 bcm2835_delay(500);
 }
 }
 else if (isFire && !isSmoke)
 {
 uint8_t i = 4;
 while(i-->0)
 {
 bcm2835_gpio_write(BUZPIN, HIGH);
 bcm2835_delay(500);
 bcm2835_gpio_write(BUZPIN, LOW);
 bcm2835_delay(500);
 }
 }

 bcm2835_delay(100);
 isFire = false;
 isSmoke = false;

 }

 camera.release();
 }

 /*
 string msg1 = "Yangın var";
 */

 bcm2835_i2c_end();
 bcm2835_close();
 return 0;
}