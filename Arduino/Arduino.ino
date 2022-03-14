/****Khai báo thư viện****/
#include <Wire.h>
#include <AccelStepper.h>

/****Định nghĩa chân cảm biến:****/
#define TRIGGER 11
#define ECHO 10

/****Định nghĩa chân động cơ:****/
// IN1 trên trình điều khiển ULN2003
#define motorPin1 4

// IN2 trên trình điều khiển ULN2003
#define motorPin2 5

// IN3 trên trình điều khiển ULN2003
#define motorPin3 6

// IN4 trên trình điều khiển ULN2003
#define motorPin4 7

/****Xác định kiểu giao diện AccelStepper****/
// Động cơ 4 dây ở chế độ nửa bước:
#define MotorInterfaceType 8

/*Khởi tạo với trình tự chân IN1-IN3-IN2-IN4 để sử dụng
thư viện AccelStepper với động cơ bước 28BYJ-48:*/
AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

/****Khỏi tạo các biến globle****/
unsigned long time_start = 0, time_end = 0, time_counter = 0, duration_var = 0, distance_var = 20;
String status_var = "";
int auto_tmp = 0, manual_tmp = 0, request = 0, confirm_var = 0, denta_t = 3000, target_position = 4096, speed_var = 1000; 
void setup()
{
  /****Cảm biến sóng âm****/
  // chân thu tín hiệu
  pinMode(ECHO, INPUT);
  // chân phát tín hiệu
  pinMode(TRIGGER, OUTPUT);

  /****Động cơ bước****/
  // Tốc độ tối đa (bước/s)
  stepper.setMaxSpeed(1000);
  // Gia tốc (bước/s/s)
  stepper.setAcceleration(1000);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  /****Giao tiếp I2C****/
  // Tham gia vào I2C Bus với địa chỉ 8
  Wire.begin(8);
  // Đăng ký nhận sự kiện
  Wire.onReceive(receiveEvent);
  // Đăng ký sự kiện yêu cầu
  Wire.onRequest(requestEvent);

  /****Start serial for debug ****/
  Serial.begin(9600);
}

void distance()
{
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  /****************************************************************************
  Tốc độ của sóng siêu âm trong không khí lý tưởng là 340 m/s (hằng số vật lý),
  tương đương với 29,412 microSeconds/cm. Khi đã tính được thời gian, ta sẽ
  chia cho 29,412 để nhận được khoảng cách.
  ****************************************************************************/
  duration_var = pulseIn(ECHO, HIGH);
  distance_var = (duration_var / 2) / 29.1;
  delay(500);
}

// Hàm thực thi bất cứ khi nào dữ liệu được nhận từ master
void receiveEvent() {}

// Hàm thực thi bất cứ khi nào dữ liệu được yêu cầu từ master
void requestEvent()
{
  if (status_var == "open")
  {
    if (time_counter > 0 && time_counter <= denta_t)
    {
      Wire.write(1);
    }
    else
    {
      Wire.write(-1);
    }
  }
  if (status_var == "close")
  {
    if (time_counter > 0 && time_counter <= denta_t)
    {
      Wire.write(2);
    }
    else
    {
      Wire.write(-2);
    }
  }
}

void openAction()
{
  stepper.setCurrentPosition(0);
  time_start = millis();
  while (stepper.currentPosition() != target_position)
  {
    stepper.setSpeed(speed_var);
    stepper.runSpeed();
  }
  time_end = millis();
  time_counter = time_end - time_start;
  status_var = "open";
}

void closeAction()
{
  stepper.setCurrentPosition(0);
  time_start = millis();
  while (stepper.currentPosition() != -target_position)
  {
    stepper.setSpeed(-speed_var);
    stepper.runSpeed();
  }
  time_end = millis();
  time_counter = time_end - time_start;
  status_var = "close";
}

void manualMode()
{
  if (request == 1 && manual_tmp == 0)
  {
    openAction();
    manual_tmp = 1;
  }
  else if (request == 255 && manual_tmp == 1)
  {
    closeAction();
    manual_tmp = 0;
  }
}

void autoMode()
{
  if (request == 2)
  {
    distance();
    Serial.println(distance_var);
    // Open
    if (distance_var <= 15)
    {
      if (auto_tmp == 0)
      {
        openAction();
      }
      auto_tmp = 1;
    }
    // Close
    if (distance_var > 15)
    {
      if (auto_tmp == 1)
      {
        closeAction();
      }
      auto_tmp = 0;
    }
  }
  else if (request == 254)
  {
    if (status_var == "open")
    {
      closeAction();
    }
    auto_tmp = 0;
  }
}

void faceRecognitionMode()
{
  Serial.println(confirm_var);
  Serial.println(request);
  if (request == 3 && confirm_var == 4)
  {
    openAction();
    closeAction();
    confirm_var = 0;
  }
  else if (request == 253)
  {
    if (status_var == "open")
    {
      closeAction();
    }
  }
}

void loop()
{
  if (Wire.available())
  {
    int req = Wire.read();
    if(req != 0){
      if (req == 4)
      {
        confirm_var = req;
      }
      else{
        request = req;
      }
    }
  }
  manualMode();
  autoMode();
  faceRecognitionMode();
  time_start = time_end = 0;
}
