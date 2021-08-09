#include <iostream>
#include <conio.h>
#include <thread>

using namespace std;
using namespace std::literals::chrono_literals;
using std::cout;

const unsigned int default_tank_volume = 60;
const unsigned int min_fuel_lvl = 5;
const double default_engine_consumption = 6;
const unsigned int default_max_speed = 250;
const unsigned int default_acceleration = 10;

bool started;

enum Keys
{
	Escape = 27,
	Enter = 13,
	Space = 32,
	ArrowUp = 72,
	ArrowDown = 80,
	ArrowLeft = 75,
	ArrowRight = 77
};

class Tank
{
	// Топливный бак:
	const unsigned int volume; // объём бака
public:
	double fuel_level; // Уровень топлива
	unsigned int get_volume()const
	{
		return volume;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}

	void fill(double amount)
	{
		// amount - кол-во топлива
		if (amount < 0)return;
		if (fuel_level + amount < volume)fuel_level += amount;
		else fuel_level = volume;
	}
	double give_fuel(double amount)
	{
		if (fuel_level - amount > 0)fuel_level -= amount;
		else fuel_level = 0;
		return fuel_level;
	}
	Tank(const unsigned int volume) :volume(volume >= 20 && volume <= 80 ? volume : default_tank_volume), fuel_level(0)
	{
		cout << "Tank is ready\t" << this << endl;
	}
	~Tank()
	{
		cout << "Tank is over\t" << this << endl;
	}
	
	void info()const
	{
		cout << "Tank volume: " << volume << " liters\n";
		cout << "Fuel level: " << fuel_level << " liters\n";
	}
};

class Engine
{
	// Этот класс описывает двигатель
	const double consumption; // Расход 100 км
	double consumption_per_second; // Расход в секунду

	

	

public:
	const double get_consumption()const
	{
		return consumption;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	void set_consumption_per_second(double consumption)
	{
		if (consumption >= .0003 && consumption <= .003)
		{
			this->consumption_per_second = consumption;
		}
	}
	bool is_started()const
	{
		return started;
	}

	void start()
	{
		started = true;
	}
	void stop()
	{
		started = false;
	}
	Engine(double consumption) :consumption(consumption >= 4 && consumption <= 25 ? consumption : default_engine_consumption)
	{
		this->consumption_per_second = this->consumption*5e-5;
		started = false;
		cout << "Engine is ready\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over\t" << this << endl;
	}
	void info()const
	{
		cout << "Engine consumes: " << consumption << " liters per 100km" << endl;
		cout << "Engine consumes: " << consumption_per_second << " per second in idle" << endl;
		cout << "Engine is " << (started ? "started" : "stoped") << endl;
	}
};

class Car
{
	Tank tank;
	Engine engine;
	unsigned int speed; // Текущая скорость
	unsigned int max_speed;
	unsigned int acceleration;

	bool driver_inside;

	struct Control
	{
		thread main_thread;
		thread panel_thread;
		thread idle_thread;
		thread free_wheeling_thread;
	}control;

public:
	/*Car(unsigned int max_speed) :
		speed(0),
		max_speed(max_speed >= 100 && max_speed <= 350 ? max_speed : default_max_speed),
		engine(this->max_speed / 15), 
		tank(this->max_speed / 5)
	{
		
	}*/
	Car(double engine_consumption, unsigned int tank_volume, unsigned int max_speed = default_max_speed) :
		engine(engine_consumption), 
		tank(tank_volume),
		speed(0), 
		max_speed(max_speed >= 100 && max_speed <= 350 ? max_speed : default_max_speed),
		acceleration(default_acceleration),
		driver_inside(false)
	{
		cout << "Your car is ready to go. Press Enter to get in." << this << endl;
	}
	~Car()
	{
		cout << "Your car is over :((((999((99\t" << this << endl;
	}

	void get_in()
	{
		driver_inside = true;
		control.panel_thread = thread(&Car::panel, this); // Запускаем метод panel в потоке
		//control.idle_thread = thread(&Car::idle, this); // По идее запускаю метод идле в потоке
	}
	void get_out()
	{
		if (speed > 20)throw exception("You left ur car on high speed");
		driver_inside = false;
		if (control.panel_thread.joinable())control.panel_thread.join(); // Останавливаем поток, отображающий панель приборов
		
	}
	void panel()const
	{

		while (driver_inside)
		{
			system("CLS");
			cout << "_____________________________________________" << endl;
			cout << "| Engine is " << (engine.is_started() ? "started" : "stopped") << endl;
			cout << "| Fuel lvl: " << tank.get_fuel_level() << " liters";
			if (tank.get_fuel_level() < min_fuel_lvl)cout << " Low fuel";
			cout << endl;
			cout << "| Engine consumption per second: " << engine.get_consumption_per_second() << endl;
			cout << "| Speed: " << speed << " hm/h" << endl;
			cout << "_____________________________________________" << endl;
			this_thread::sleep_for(1s);
		}
		system("CLS");
		cout << "This is your car, press Enter to get in" << endl;
	}
	void start()		// Заводит машину
	{
		if (driver_inside && tank.get_fuel_level() > 0)
		{
			engine.start();
			control.idle_thread = thread(&Car::engine_idle, this);
		}
	}
	void stop()			// Глушит двигатель
	{
		engine.stop();
		if (control.idle_thread.joinable())control.idle_thread.join();
	}
	void engine_idle()	// Расходует бензин на холостом ходу
	{
		while (tank.give_fuel(engine.get_consumption_per_second()) && engine.is_started())
		{
			this_thread::sleep_for(1s);
		}
		engine.stop();
	}

	void free_wheeling()
	{
		while (speed > 0)
		{
			speed--;
			if (speed < 0)speed = 0;
			this_thread::sleep_for(1s);
		}
	}

	void accelerate()
	{
		if (engine.is_started() && speed < max_speed)
		{
			speed += acceleration;
			if (control.free_wheeling_thread.get_id() == thread::id())control.free_wheeling_thread = thread(&Car::free_wheeling, this);
			adjust_consumption();
			
		}
		this_thread::sleep_for(1s);
	}
	
	void slow_down()
	{
		if (speed > 0)
		{
			speed -= acceleration;
			if (speed < acceleration)
			{
				speed = 0;
				if (control.free_wheeling_thread.joinable())
				{
					control.free_wheeling_thread.join();
				}
			}
			adjust_consumption();
		}
		this_thread::sleep_for(1s);
	}

	void adjust_consumption()
	{
		if (speed > 0 && speed < 60)engine.set_consumption_per_second(.002);
		else if (speed > 60 && speed <= 100)engine.set_consumption_per_second(.0014);
		else if (speed > 100 && speed <= 140)engine.set_consumption_per_second(.002);
		else if (speed > 140 && speed <= 200)engine.set_consumption_per_second(.0025);
		else if (speed > 200)engine.set_consumption_per_second(.003);
		else if (speed == 0)engine.set_consumption_per_second(engine.get_consumption()*5e-5);
	}

	void control_car()
	{
		char key;
		do
		{
			key = _getch();
			switch (key)
			{
			case Enter: // Вход или выход из машины
				if (driver_inside)get_out();
				else get_in();
				break;
			case 'F': // Fill - заправка
			case 'f':
				double amount;
				cout << "How much?"; cin >> amount;
				tank.fill(amount);
				break;
			case 'I': // Ignition - зажигание
			case 'i':
				if (engine.is_started())stop();
				else start();
				break;
			case 'W':case 'w':case ArrowUp: // Gas - газ, разгон
				accelerate();
				break;
			case ArrowDown:case Space:case 'S':case 's':
				slow_down();
				break;
			case Escape:
				stop();
				get_out();
			}
			if (tank.get_fuel_level() == 0 && control.idle_thread.joinable())control.idle_thread.join();
		} while (key != Escape);
	}

	void info()const
	{
		tank.info();
		engine.info();
		cout << "Max speed:\t" << max_speed << "km/h" << endl;
		cout << "Current speed: " << speed << "km/h" << endl;
	}
};

//#define tank_check
//#define engine_check

int main()
{
	setlocale(LC_ALL, "Russian");
	system("color 0A");
#ifdef tank_check

	Tank tank(11);
	tank.info();
	tank.fill(3);
	cout << endl;
	tank.info();
	tank.fill(30);
	cout << endl;
	tank.info();
	tank.fill(30);
	cout << endl;
	tank.info();
#endif // tank_check

#ifdef engine_check
	Engine engine(6);
	engine.info();
	engine.start();
	engine.info();
#endif // engine_check

	Car car(8, 40);
	car.info();
	try
	{
		car.control_car();
	}
	catch (const std::exception& e)
	{
		cerr << e.what() << endl;
	}
}
