#include "ofApp.h"

#include "kuMenu.h"
#include "kusProject.h"
#include "kusState.h"
#include "pbFiles.h"
#include "kuWavFile.h"
#include "kuNavigator.h"
#include "kuButton.h"
#include "kuConsole.h"

string PRODUCT_NAME = "KuStudio (1.78)";

kuMenu menu;
kuNavigator navi;
kuButtons buttons;
kuButton *b_playall, *b_play, *b_stop;
kuButton *b_edit, *b_edit_line;
kuButton *b_record;

int buttonX = 350;
int buttonY = 10;
int timeTextX = buttonX + 140;
int buttonX2 = timeTextX + 110;


void *mouseState = 0;

kusProject project;
ofTrueTypeFont font2;

//--------------------------------------------------------------
void ofApp::setup() {
	console_hide();	//Hide console in Windows

	ofSetWindowTitle(PRODUCT_NAME);
	ofSetEscapeQuitsApp(false);   //выключили ESC
	ofSetVerticalSync(false); //отключили вертикальную синхронизацию

	//состояние
	shared_state.setup();

	float winw = ofGetWidth();
	float winh = ofGetHeight();

	string fontFile = "fonts/verdana.ttf";
	int fontSize = 10;
	int fontSize2 = 15;
	int fontSize3 = 8;
	font2.loadFont(fontFile, fontSize2);

	menu.setup(fontFile, fontSize, 5, 5, 10, 5);

	menu.addSubmenu("proj", "Project");
	menu.addItem("projNew", "New", "proj");
	menu.addItem("projOpen", "Open...", "proj");
	menu.addItem("projSave", "Save", "proj");
	menu.addItem("projSaveAs", "Save As...", "proj");
	//menu.addItem( "projDuration", "Set duration...", "proj" );
	menu.addItem("projAudio", "Set audio track...", "proj");
	menu.addItem("projExportText", "Export as text file...", "proj");
	menu.addItem("projImportText", "Import text file...", "proj");
	menu.addItem("projShiftAllTracks_sec", "Shift all tracks, seconds...", "proj");
	menu.addItem("projShiftAllTracks_frames", "Shift all tracks, frames...", "proj");

	menu.addItem("projQuitNoSave", "Quit without saving", "proj");
	menu.addSubmenu("tr", "Track");
	menu.addItem("trAddFloat", "New float track...", "tr");
	menu.addItem("trAddInt", "New int track...", "tr");
	menu.addItem("trName", "Name...", "tr");
	menu.addItem("trOscOut", "OSC out...", "tr");
	menu.addItem("trRange", "Range...", "tr");
	menu.addItem("trSmooth", "Smooth...", "tr");
	menu.addItem("trType", "Type...", "tr");

	menu.addItem("trDelete", "Delete track...", "tr");
	menu.addItem("trEdit", "(For track drawing choose it and press Z)", "tr");

	menu.addItem("trShiftTrack_sec", "Shift track fragment, seconds...", "tr");
	menu.addItem("trShiftTrack_frames", "Shift track fragment, frames...", "tr");

	//Конвертирует продолжительные пики на int-треках в один отсчет - удобно для редактирования событий
	menu.addItem("trKeepIntPeaksOnly", "Keep peaks only at all int tracks", "tr");

	//выравнивает растояния между событиями во фрагменте
	menu.addItem("trUniformFragment", "Make uniform events at int track fragment (Shift+U)", "tr");


	menu.addSubmenu("snd", "Sound");
	menu.addItem("sndVolMute", "Mute", "snd");
	menu.addItem("sndVol10", "Volume 10%", "snd");
	menu.addItem("sndVol25", "Volume 25%", "snd");
	menu.addItem("sndVol50", "Volume 50%", "snd");
	menu.addItem("sndVol75", "Volume 75%", "snd");
	menu.addItem("sndVol100", "Volume 100%", "snd");
	menu.addItem("sndSpeed25", "Speed 25%", "snd");
	menu.addItem("sndSpeed33", "Speed 33%", "snd");
	menu.addItem("sndSpeed50", "Speed 50%", "snd");
	menu.addItem("sndSpeed100", "Speed 100%", "snd");

	menu.addSubmenu("sett", "Settings");
	menu.addItem("settOscOut1", "OSC out 1", "sett");
	menu.addItem("settOscOut2", "OSC out 2", "sett");
	menu.addItem("settOscOut3", "OSC out 3", "sett");
	menu.addItem("settOscOut4", "OSC out 4", "sett");
	menu.addItem("settOscIn", "OSC in", "sett");
	menu.addItem("trRate", "(Tracks frame rate: " + ofToString(kusTrack::tracks_rate()) + " fps)", "sett");
	menu.addItem("settFPS", "(App. frame rate: " + ofToString(shared_state.FPS) + " fps - settings.ini)", "sett");

	menu.addSubmenu("help", "?");
	menu.addItem("help", "Help...", "help");


	//menu.addItem( "settPlayerOut1", "kuPlayer 1", "sett" );
	//menu.addItem( "settPlayerOut2", "kuPlayer 2", "sett" );
	//menu.addItem( "settPlayerOut3", "kuPlayer 3", "sett" );
	//menu.addItem( "settPlayerOut4", "kuPlayer 4", "sett" );


	navi.setup(3, 55, winw - 6, 30);
	buttons.setup(10);

	string icons = "icons/";
	b_playall = buttons.addButton(icons + "playall.png", buttonX, buttonY);
	b_playall->setToggle(true);
	b_play = buttons.addButton(icons + "play.png", buttonX + 40, buttonY);
	b_play->setToggle(true);
	b_stop = buttons.addButton(icons + "stop.png", buttonX + 80, buttonY);

	b_edit = buttons.addButton(icons + "edit.png", buttonX2, buttonY);
	b_edit->setToggle(true);
	b_edit_line = buttons.addButton(icons + "edit_line.png", buttonX2 + 40, buttonY);
	b_edit_line->setToggle(true);
	b_record = buttons.addButton(icons + "record.png", buttonX2 + 80, buttonY);
	b_record->setToggle(true);


	//состояние
	b_edit->setDown(false);
	b_edit_line->setDown(false);
	b_record->setDown(shared_state.recordButton);

	//проект
	project.setup(winw, winh, fontFile, fontSize, fontFile, fontSize3);
	project.setThumbSize(navi.getW(), navi.getH());
	//загрузка последнего открытого проекта
	project.open(shared_state.lastProjectFile);
	projectWasUpdated();

	_saveOnExit = true;
}

//--------------------------------------------------------------
void ofApp::resized() {
	int minSize = 7;
	int w = max(ofGetWidth(), minSize);
	int h = max(ofGetHeight(), minSize);
	navi.resized(w - 6, 30);
	project.setThumbSize(navi.getW(), navi.getH());
	project.setWindowSize(w, h);
}

//--------------------------------------------------------------
void ofApp::exit() {
	cout << "Exiting..." << endl;
	if (_saveOnExit) project.save(false, false);
	shared_state.save();

}

//--------------------------------------------------------------
void ofApp::exitWithoutSaving() {
	cout << "Exiting without saving..." << endl;
	_saveOnExit = false;
	OF_EXIT_APP(0);
}

//--------------------------------------------------------------
float time_ = 0;
void ofApp::update() {
	float time = ofGetElapsedTimef();
	float dt = ofClamp(time - time_, 0.01, 0.1);
	time_ = time;

	processOsc();

	menu.update(dt);
	processMenu(menu.commandName());

	processButtons();
	buttons.update(dt);

	if (navi.isSegmentChanged()) {
		project.setViewSegment(navi.time0(), navi.time1());
	}
	if (navi.isSegmentLeftChanged()) {
		project.requestSendTracksValues();
	}
	project.update(dt);

	navi.update(dt);


	ofSoundUpdate();

}

//--------------------------------------------------------------
void help() {
	ostringstream s;
	s << "<<<< " << PRODUCT_NAME << ">>>> by Denis Perevalov" << endl << "(first version was made with kuflex.com)" << endl;
	s << "How to use:" << endl;
	s << "1) Set audio track" << endl;
	s << "      Menu 'Set audio track' - select WAV or AIFF file, 8,16,32-bit," << endl << "         (use latin paths and filename)" << endl;
	s << "      First sound graph - whole audio track, second - audio fragment." << endl << " Select and drag end of fragment (white lines) on the whole track." << endl;
	s << "      Press SPACE to play/stop segment, " << endl << " ENTER to play all (stop only by pressing Stop button)" << endl;
	s << "2) Create new tracks" << endl;
	s << "      Menu 'New track' - set track name, it will be sent by OSC" << endl;
	s << "      Select track by clicking it" << endl;
	s << "      Press Z and draw by mouse; also use 'Tracks' menu for setup" << endl;
	s << "      Press X and drag for drawing straight lines" << endl;
	s << "3) OSC track in/out" << endl;
	s << "      Track receives OSC signals with name as track's name" << endl;
	s << "      Press Record button (circle) for recording data to tracks" << endl;
	s << "      Track sends OSC signals by its name; it can send several names, separated by commas" << endl;
	s << "4) Kustudio receives by OSC: " << endl;
	s << "   playStopSegment, stopSegment, playAll, stopAll" << endl;
	s << "   openProject (path_to_project)" << endl;
	s << "5) Kustudio sends by OSC: " << endl;
	s << "   kustudio:status (scene master_time app_time playAll|playSegm|stop)" << endl;
	s << "6) About export/import text files: currently track's frame rate is fixed to " << kusTrack::tracks_rate() << " fps" << endl;

	ofSystemAlertDialog(s.str());
}

//--------------------------------------------------------------
void ofApp::processMenu(string cmd) {
	if (cmd == "") return;
	//cout << cmd << endl;
	if (cmd == "projNew") { project.createNew(true); projectWasUpdated(); }
	if (cmd == "projOpen") { project.openAs(); projectWasUpdated(); }
	if (cmd == "projSave") project.save(false);
	if (cmd == "projSaveAs") project.save(true);
	if (cmd == "projExportText") project.exportRawText();
	if (cmd == "projImportText") { project.importRawText(); projectWasUpdated(); }
	if (cmd == "projShiftAllTracks_sec") project.shiftAllTracks_sec();
	if (cmd == "projShiftAllTracks_frames") project.shiftAllTracks_frames();


	//if ( cmd == "projDuration" ) { projectWasUpdated(); }
	if (cmd == "projAudio") { project.openAudioAs(); projectWasUpdated(); }
	if (cmd == "help") { help(); }

	if (cmd == "projQuitNoSave") { exitWithoutSaving(); }

	if (cmd == "trAddFloat") { project.addTrack(true); }
	if (cmd == "trAddInt") { project.addTrack(false); }
	if (cmd == "trName") { project.editTrackName(); }
	if (cmd == "trOscOut") { project.editTrackOscOut(); }
	if (cmd == "trRange") { project.editTrackRange(); }
	if (cmd == "trSmooth") { project.editTrackSmooth(); }
	if (cmd == "trType") { project.editTrackType(); }
	if (cmd == "trDelete") { project.deleteTrack(); }
	if (cmd == "trKeepIntPeaksOnly") { project.keepIntPeaksOnly(); }

	if (cmd == "trShiftTrack_sec") { project.shiftTrackFragment_sec(); }
	if (cmd == "trShiftTrack_frames") { project.shiftTrackFragment_frames(); }
	if (cmd == "trUniformFragment") { project.uniformIntTrackFragment(); }	

	if (cmd == "sndVolMute") { project.setVolume(0); }
	if (cmd == "sndVol10") { project.setVolume(0.10); }
	if (cmd == "sndVol25") { project.setVolume(0.25); }
	if (cmd == "sndVol50") { project.setVolume(0.5); }
	if (cmd == "sndVol75") { project.setVolume(0.75); }
	if (cmd == "sndVol100") { project.setVolume(1.0); }

	if (cmd == "sndSpeed25") { project.setSpeed(0.25); }
	if (cmd == "sndSpeed33") { project.setSpeed(0.33); }
	if (cmd == "sndSpeed50") { project.setSpeed(0.5); }
	if (cmd == "sndSpeed100") { project.setSpeed(1.0); }
	
	if (cmd == "settOscOut1") { project.editOscOut(0); }
	if (cmd == "settOscOut2") { project.editOscOut(1); }
	if (cmd == "settOscOut3") { project.editOscOut(2); }
	if (cmd == "settOscOut4") { project.editOscOut(3); }
	if (cmd == "settPlayerOut1") { project.editPlayerOut(0); }
	if (cmd == "settPlayerOut2") { project.editPlayerOut(1); }
	if (cmd == "settPlayerOut3") { project.editPlayerOut(2); }
	if (cmd == "settPlayerOut4") { project.editPlayerOut(3); }

	if (cmd == "settOscIn") { project.editOscIn(); }


}

//--------------------------------------------------------------
void ofApp::playAll() {
	project.playAll();
	b_play->setDown(false);
	b_play->setEnabled(false);
}

//--------------------------------------------------------------
void ofApp::playSegment() {
	project.playSegment();
}

//--------------------------------------------------------------
void ofApp::stop() {
	project.stop();
	b_playall->setDown(false);
	b_play->setDown(false);
	b_play->setEnabled(true);
}

//--------------------------------------------------------------
void ofApp::processButtons() {
	if (b_stop->isPressed()) { stop(); }

	if (b_playall->isPressed()) {
		if (b_playall->isDown()) { playAll(); }
		else { stop(); }
	}
	if (b_play->isPressed()) {
		if (b_play->isDown()) { playSegment(); }
		else { stop(); }
	}

	//проверка что исполнение кончилось
	if (b_playall->isDown() && !project.isPlayingAll()) {
		stop();
	}
	if (b_play->isDown() && !project.isPlayingSegment() && !project.isPlayingAll()) {
		stop();
	}

	//edit
	if (b_edit->isDown() != shared_state.editButton) {
		shared_state.setEditButton(b_edit->isDown());
	}
	if (b_edit_line->isDown() != shared_state.editLineButton) {
		shared_state.setEditLineButton(b_edit_line->isDown());
	}
	//record
	if (b_record->isDown() != shared_state.recordButton) {
		shared_state.setRecordButton(b_record->isDown());
	}

}

//--------------------------------------------------------------
void ofApp::processOscCommand(string cmd, vector<string> &values) {
	//cout << "Command: " << cmd << endl;
	//for (int i = 0; i < values.size(); i++) {
	//	cout << "\t" << values[i] << endl;
	//}

	if (cmd == "playStopSegment") {
		if (!project.isPlayingAll()) {
			if (project.isPlayingSegment()) stop(); else playSegment();
		}
	}
	if (cmd == "stopSegment") {
		if (!project.isPlayingAll()) stop();
	}
	if (cmd == "playAll") {
		if (!project.isPlayingAll()) playAll();
	}
	if (cmd == "stopAll") {
		stop();
	}
	if (cmd == "openProject" && values.size() >= 1) {
		string project_file = values[0];
		project.open(project_file);
		projectWasUpdated();

	}
}

//--------------------------------------------------------------
void ofApp::processOsc() {
	//обработка OSC-сообщений
	while (kus.oscIn.hasWaitingMessages()) {
		ofxOscMessage m;
		kus.oscIn.getNextMessage(m);
		string name = m.getAddress().substr(1);
		int n = m.getNumArgs();

		//если один аргумент типа float - то значение слайдера, иначе - команда
		if (n == 1 && m.getArgType(0) == OFXOSC_TYPE_FLOAT) {
			float value = m.getArgAsFloat(0);
			project.setTrackValue(name, value);
		}
		else {
			vector<string> values(n);
			for (int i = 0; i < n; i++) {
				if (m.getArgType(i) == OFXOSC_TYPE_STRING) {
					values[i] = m.getArgAsString(i);
				}
			}
			processOscCommand(name, values);
		}
	}
}


//--------------------------------------------------------------
void ofApp::projectWasUpdated() {
	navi.setDuration(project.duration);
	navi.setSegment(project.time0(), project.time1());

	ofSetWindowTitle(PRODUCT_NAME + " - " + project.name()
		+ ", oscin: " + ofToString(kus.oscIn.port()));
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(64);

	project.draw();

	//навигация -----
	//весь звуковой файл
	if (!project.isPlayingAll()) {
		ofSetColor(128);
	}
	else {
		ofSetColor(255);
	}
	project.drawThumb(navi.getX(), navi.getY());

	navi.draw();
	//текущее время полоской в навигации
	float timeX = navi.timeToPix(project.time());
	ofSetColor(255, 255, 255);
	ofLine(timeX, navi.getY() - 8, timeX, navi.getY() + navi.getH());
	//----------------
	//крестик мыши
	bool edit = (b_edit->isDown() || b_edit_line->isDown());
	if (edit) {
		ofSetColor(219, 160, 84, (edit) ? 255 : 128);
		ofLine(mouseX, kTracksAudioY - 8, mouseX, ofGetHeight());
		if (mouseY >= kTracksY) {
			ofLine(0, mouseY, ofGetWidth(), mouseY);
		}
	}

	//----------------

	//текущее время текстом
	ofSetColor(200);
	font2.drawString(project.timeString(), timeTextX, 30);


	menu.draw();
	buttons.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_RETURN) { b_playall->setPressed(); }
	if (key == ' ') { b_play->setPressed(); }
	if (key == 'z') { b_edit->setDown(true); }
	if (key == 'x') { b_edit_line->setDown(true); }

	if (key == 'U') { //выравнивание событий на треке
		processMenu("trUniformFragment");
	}

	//    bool ctrl = ofGetKeyPressed(OF_KEY_CONTROL);
	//    cout << "ctrl " << ctrl << endl;
	//    bool alt = ofGetKeyPressed(OF_KEY_ALT);
	//    cout << "alt " << alt << endl;

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	if (key == 'z') { b_edit->setDown(false); }
	if (key == 'x') { b_edit_line->setDown(false); }

}

//--------------------------------------------------------------
//обработка событий мыши
//type 0 - move, 1 - press, 2 - drag, 3 - release
//mouseState - указатель, использует ли кто-то мышь
void ofApp::mouse(int x, int y, int button, void *&mouseState, int type) {
	menu.mouse(x, y, button, mouseState, type);
	buttons.mouse(x, y, button, mouseState, type);
	navi.mouse(x, y, button, mouseState, type);
	project.mouse(x, y, button, mouseState, type);
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	//mouse( x, y, 0, mouseState, 0 ); //пока выключил
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	mouse(x, y, button, mouseState, 1);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	mouse(x, y, button, mouseState, 2);
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	mouse(x, y, button, mouseState, 3);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	resized();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

