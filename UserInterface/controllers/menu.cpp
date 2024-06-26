//
// Created by independent-variable on 4/5/2024.
//
#include "../controllers.h"
#include "../display.h"
#include "../symbols.h"

static const char down = '\0', up = '\1', enter = '\2', back = '\3';

namespace user_interface {
	class Menu : public Controller {
	private:
		uint8_t item;
		const uint8_t first_item = 0, last_item = 4;
		const uint8_t up_pos = 0, down_pos = 4;

		/** Expects that cursor is at the beginning of the second line */
		inline void show_item() {
			switch (item) {
				case 0:
					disp.define('\4', symbol::ua_U).define('\5', symbol::ua_MILD);
					disp << " AKT\4BHICT\5 3OH ";
					break;
				case 1:
					disp.define('\4', symbol::ua_L).define('\5', symbol::ua_SH);
					disp << "  HA\4A\5T. 3OH   ";
					break;
				case 2:
					disp.define('\4', symbol::ua_P).define('\5', symbol::ua_L).define('\6', symbol::ua_YU);
					disp << " 3AMIHA \4APO\5\6  ";
					break;
				case 3:
					disp.define('\4', symbol::ua_L).define('\5', symbol::ua_F).define('\6', symbol::ua_U);
					disp << "    TE\4E\5OH\6    ";
					break;
				case 4:
					disp.define('\4', symbol::ua_U).define('\5', symbol::ua_P).define('\6', symbol::ua_C);
					disp << "3ATP\4MKA C\5PA\6. ";
					break;
			}
		}
	protected:
		void activate(bool init) override;
		void handle(keyboard::Button button, keyboard::Event event) override;
	};

	static Menu menu_inst;
	Controller * const menu = &menu_inst;
}

using namespace user_interface;

void Menu::activate(bool init) {
	if (init) {
		item = 0;
	}
	disp.clear()
			.define(down, symbol::down)
			.define(up, symbol::up)
			.define(enter, symbol::enter)
			.define(back, symbol::exit)
			.set_cursor(0, 0);

	// show 1st line
	if (item != first_item) {
		disp[up_pos] << "A:" << up;
	}
	if (item != last_item) {
		disp[down_pos] << "B:" << down;
	}
	disp[9] << "C:" << enter << ' ' << "D:" << back;
	// show 2nd line
	disp.set_cursor(1, 0);
	show_item();
	disp.set_cursor(1, 0);
}

void Menu::handle(keyboard::Button button, keyboard::Event event) {
	using keyboard::Button, keyboard::Event;
	if (button == Button::A && event == Event::CLICK) {
		// move up
		if (item == first_item) {
			return;
		}
		uint8_t prev_item = item--;
		show_item();
		if (item == first_item) {
			// hide 'up'
			disp.set_cursor(0, up_pos).print("   ");
		} else if (prev_item == last_item) {
			// show 'down'
			disp.set_cursor(0, down_pos).print("B:").print(down);
		}
		disp.set_cursor(1, 0);
	} else if (button == Button::B && event == Event::CLICK) {
		// move down
		if (item == last_item) {
			return;
		}
		uint8_t prev_item = item++;
		show_item();
		if (item == last_item) {
			// hide 'down'
			disp.set_cursor(0, down_pos).print("   ");
		} else if (prev_item == first_item) {
			// show 'up'
			disp.set_cursor(0, up_pos).print("A:").print(up);
		}
		disp.set_cursor(1, 0);
	} else if (button == Button::C && event == Event::CLICK) {
		// enter
		switch (item) {
			case 0:
				invoke(zone_viewer);
				break;
			case 1:
				invoke(zone_configurer);
				break;
			case 2:
				invoke(password_editor);
				break;
			case 3:
				invoke(phone_configurer);
				break;
			case 4:
				invoke(delay_editor);
				break;
		}
	} else if (button == Button::D && event == Event::CLICK) {
		// back
		yield();
	}
}