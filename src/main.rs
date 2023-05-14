use raylib::prelude::*;
use serde::{ Deserialize, Serialize };

#[allow(non_snake_case)]
#[derive(Debug, Deserialize)]
struct Sheet {
    range: String,
    majorDimension: String,
    values: Vec<Vec<String>>,
}

#[allow(non_snake_case)]
#[derive(Debug, Serialize)]
struct Team {
    name: String,
    score: i32,
	color: Color,
}

fn main() {
    let response = reqwest::blocking
        ::get(
            "https://sheets.googleapis.com/v4/spreadsheets/1JXpmjPBp9jPgNV9JzadyE0ZJ1xovF4pDf5BpP50aYlg/values/Sheet1!A:B?key=AIzaSyDdPUw7fL6B3tduJKHV9NyU6wveOl3Re-U"
        )
        .unwrap();
    let sheet: Result<Sheet, reqwest::Error> = response.json();

    let teams: Vec<Team> = sheet
        .unwrap()
        .values.iter()
        .map(|v| Team {
            name: v[0].clone(),
            score: v[1].parse().unwrap(),
        })
        .collect();
    println!("{:?}", teams);

	let (mut rl, thread) =raylib::init()
		.size(1800, 1000)
		.title("Many-Team Scoreboard")
		.vsync()
		.build();

	while !rl.window_should_close() {
		let mut d = rl.begin_drawing(&thread);

		d.clear_background(Color::WHITE);
		d.draw_text("Many-Team Scoreboard", 10, 10, 20, Color::BLACK);
		// Draw each team name and score (1 team per row)
		for (i, team) in teams.iter().enumerate() {
			// define font size
			let font_size = 60;
			d.draw_text(&team.name, 10, 40 + i as i32 * font_size, font_size, Color::BLACK);
			d.draw_text(&team.score.to_string(), 1000, 40 + i as i32 * font_size, font_size, Color::BLACK);
			// Draw a line under each team
			//d.draw_line(10, 40 + i as i32 * font_size, 790, 40 + i as i32 * 20, Color::BLACK);
			// Draw a box around each team
			//d.draw_rectangle(10, 40 + i as i32 * 20, 790, 40 + i as i32 * 20, Color::BLACK);
		}
	}
}