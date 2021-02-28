#pragma once
class main_c {
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;

	cam_info cam_easy;
	VECTOR_ref eyevec, eyevec2;																	//視点
	FontHandle font12;
	FontHandle font18;
	//描画スクリーン
	GraphHandle outScreen, outScreen2;
	GraphHandle UI_Screen, UI_Screen2;
	MV1 cockpit;	//コックピット
	MV1 garage;
	//操作
	Mainclass::CAMS cam_s;
	float range = 0.f, range_p = 30.f;
	float fovs = 1.f, fovs_p = 1.f;
	VECTOR_ref eye_pos_ads = VGet(0, 0.58f, 0);
	VECTOR_ref HMDpos, rec_HMD;
	MATRIX_ref HMDmat, rec_HMDmat;
	//データ
	std::vector<Mainclass::Chara> chara;	//キャラ
	std::vector<Mainclass::Ammos> Ammo;		//弾薬
	std::vector<Mainclass::Vehcs> Vehicles;	//車輛データ
	Mainclass::sounds_3D se;

	SoundHandle se_alert;
	SoundHandle se_alert2;
	SoundHandle se_timer;
	SoundHandle bgm_title;
	SoundHandle bgm_main;
	SoundHandle bgm_win;

	float danger_height = 300.f;
	float se_vol = 0.35f;
	class voice_strs {
	public:
		std::string str;
		SoundHandle *handle;
	};
	class voices {
	public:
		std::vector <std::string> str;
		std::vector<SoundHandle> handle;
		float timer = 0.f;
		int select = 0;

		void set(float tm, std::vector<voice_strs>& voice_str) {
			this->select = GetRand(int(this->handle.size() - 1));
			if (CheckSoundMem(this->handle[this->select].get()) != TRUE && this->timer == 0.f) {
				this->handle[this->select].play(DX_PLAYTYPE_BACK, TRUE);
				this->handle[this->select].vol(255);
				this->timer = tm;
				voice_str.resize(voice_str.size() + 1);
				voice_str.back().handle = &this->handle[this->select];
				voice_str.back().str = this->str[this->select];
			}
		}
	};
	std::vector<voices> voice_;
	std::vector<voice_strs> voice_str;
	//timer
	float timer = 0.f;
	float ready_timer = 0.f;
	//設定
	bool oldv = false;
	bool start_c = true;
	bool start_c2 = true;
	bool ending = true;
	bool ending_win = true;
	float fov_pc = 45.f;
	//
	Mainclass::key_bind k_;
public:
	main_c() {
		//設定読み込み
		{
			SetOutApplicationLogValidFlag(FALSE);	//log
			int mdata = FileRead_open("data/setting.txt", FALSE);
			dof_e = getparams::_bool(mdata);
			bloom_e = getparams::_bool(mdata);
			shadow_e = getparams::_bool(mdata);
			useVR_e = getparams::_bool(mdata);
			fov_pc = getparams::_float(mdata);
			FileRead_close(mdata);
			SetOutApplicationLogValidFlag(TRUE);	//log
		}
		//
		auto Drawparts = std::make_unique<DXDraw>("TankFlanker", FRAME_RATE, useVR_e, shadow_e);						//汎用クラス
		auto UIparts = std::make_unique<UI>(Drawparts->disp_x, Drawparts->disp_y);										//UI
		auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);															//デバッグ
		auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->disp_x, Drawparts->disp_y);	//ホストパスエフェクト
		UI_Screen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);										//VR、フルスクリーン共用UI
		auto Hostpass2parts = std::make_unique<HostPassEffect>(dof_e, bloom_e, deskx, desky);							//ホストパスエフェクト
		UI_Screen2 = GraphHandle::Make(deskx, desky, true);																//フルスクリーン向けUI
		auto mapparts = std::make_unique<Mapclass>();																	//map
		outScreen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);	//描画スクリーン
		outScreen2 = GraphHandle::Make(deskx, desky, true);	//描画スクリーン
		//
		font12 = FontHandle::Create(y_r(12), DX_FONTTYPE_EDGE);
		font18 = FontHandle::Create(18, DX_FONTTYPE_EDGE);
		//その他
		se.Load();
		se_alert = SoundHandle::Load("data/audio/alert.wav");
		se_alert2 = SoundHandle::Load("data/audio/alert2.wav");
		se_timer = SoundHandle::Load("data/audio/timer.wav");
		bgm_title = SoundHandle::Load("data/audio/BGM/title.wav");
		bgm_main = SoundHandle::Load(std::string("data/audio/BGM/bgm") + std::to_string(GetRand(3)) + ".wav");
		bgm_win = SoundHandle::Load("data/audio/BGM/win.wav");
		//
		k_.load_keyg();
		{
			WIN32_FIND_DATA win32fdt;
			HANDLE hFind;
			int pt = -1;
			hFind = FindFirstFile("data/audio/voice/*", &win32fdt);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (win32fdt.cFileName[0] != '.') {
						int ttm = win32fdt.cFileName[0] - '0';
						if (pt != ttm) {
							voice_.resize(voice_.size() + 1);
						}
						pt = ttm;
						voice_.back().handle.resize(voice_.back().handle.size() + 1);
						voice_.back().handle.back() = SoundHandle::Load(std::string("data/audio/voice/") + win32fdt.cFileName);
						voice_.back().str.resize(voice_.back().str.size() + 1);
						voice_.back().str.back() = win32fdt.cFileName;
						voice_.back().str.back() = voice_.back().str.back().substr(2, voice_.back().str.back().find('.') - 2);
					}
				} while (FindNextFile(hFind, &win32fdt));
			} //else{ return false; }
			FindClose(hFind);
		}
		//
		Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles, true);
		MV1::Load("data/model/cockpit/model.mv1", &cockpit, true);
		//
		UIparts->load_window("車両モデル");					//ロード画面
		//
		Mainclass::Ammos::set_ammos(&Ammo);							//弾薬
		Mainclass::Vehcs::set_vehicles(&Vehicles);					//車輛
		//ラムダ
		auto shadow_draw = [&]() {
			for (auto& c : chara) {
				c.vehicle.draw();
			}
		};
		auto ram_draw = [&]() {
			Drawparts->Draw_by_Shadow(
				[&]() {
				//マップ
				mapparts->draw();
				//機体
				SetFogStartEnd(0.0f, 3000.f);
				SetFogColor(128, 128, 128);
				for (auto& c : chara) {
					c.vehicle.draw();
				}
				SetFogEnable(FALSE);
				SetUseLighting(FALSE);
				for (auto& c : chara) {
					for (auto& cg : c.vehicle.Gun_) {
						cg.draw();
					}
				}
				SetUseLighting(TRUE);
				SetFogEnable(TRUE);
			}
			);
		};
		auto mouse_aim = [&](VECTOR_ref& eyevec_tmp) {
			int mousex, mousey;
			GetMousePoint(&mousex, &mousey);
			SetMousePoint(deskx / 2, desky / 2);

			float y = atan2f(eyevec_tmp.x(), eyevec_tmp.z()) + deg2rad(float(mousex - deskx / 2) * 0.1f / fovs);
			float x = atan2f(eyevec_tmp.y(), std::hypotf(eyevec_tmp.x(), eyevec_tmp.z())) + deg2rad(float(mousey - desky / 2) * 0.1f / fovs);
			x = std::clamp(x, deg2rad(-80), deg2rad(45));
			eyevec_tmp = VGet(cos(x) * std::sin(y), std::sin(x), std::cos(x) * std::cos(y));
		};
		//ココから繰り返し読み込み//-------------------------------------------------------------------
		do {
			//読み出し
			chara.resize(24);
			auto& mine = chara[0];
			//キャラ選択
			Drawparts->reset_HMD();
			{
				//
				float speed = 0.f;
				VECTOR_ref pos;
				pos.y(1.8f);
				VECTOR_ref pos_mine = VGet(15.f, 0, 0);
				bool endp = false;
				float rad = 0.f;
				float yrad_m = 0.f, xrad_m = 0.f;
				float yrad_im = 0.f, xrad_im = 0.f;
				float ber_r = 0.f;
				int anime = 0;
				//VR
				switchs rt, lt;
				//
				int m_x = 0, m_y = 0;
				bool startp = false;
				auto& veh = mine.vehicle;
				{
					fovs = 1.f;
					bgm_title.play(DX_PLAYTYPE_LOOP, TRUE);
					bgm_title.vol(int(float(255)*0.5f));
					MV1::Load("data/model/garage/model.mv1", &garage, false);
					veh.use_id %= Vehicles.size(); //飛行機
					anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
					MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
					GetMousePoint(&m_x, &m_y);
					cam_s.cam.campos = VGet(0.f, 0.f, -15.f);
					cam_s.cam.camvec = VGet(0.f, 3.f, 0.f);
				}
				while (ProcessMessage() == 0) {
					const auto waits = GetNowHiPerformanceCount();
					if (!startp) {
						if (Drawparts->use_vr) {
							auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
							if (&ptr_LEFTHAND != nullptr) {
								if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
									//
									{
										lt.get_in(((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) && ptr_LEFTHAND.touch.x() < -0.5f);
										rt.get_in(((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) && ptr_LEFTHAND.touch.x() > 0.5f);
										if (lt.push()) {
											MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
											++veh.use_id %= Vehicles.size();
											anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
											MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
										}
										if (rt.push()) {
											MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
											if (veh.use_id == 0) {
												veh.use_id = Vehicles.size() - 1;
											}
											else {
												--veh.use_id;
											}
											anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
											MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
										}
									}
									//
									if ((ptr_LEFTHAND.on[0] & BUTTON_TRIGGER) != 0) {
										break;
									}
									//
								}
							}
						}
						else {
							{
								int x, y;
								GetMousePoint(&x, &y);
								yrad_im = std::clamp(yrad_im + float(m_x - x) / 5.f, -120.f, -30.f);
								xrad_im = std::clamp(xrad_im + float(m_y - y), -0.f, 45.f);
								m_x = x;
								m_y = y;
								easing_set(&yrad_m, deg2rad(yrad_im), 0.9f);
								easing_set(&xrad_m, deg2rad(xrad_im), 0.9f);
							}
							//
							{
								if (k_.key_use_ID[3].get_key(2)) {
									MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
									++veh.use_id %= Vehicles.size();
									anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
									MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
								}
								if (k_.key_use_ID[2].get_key(2)) {
									MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
									if (veh.use_id == 0) {
										veh.use_id = Vehicles.size() - 1;
									}
									else {
										--veh.use_id;
									}
									anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
									MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
								}
							}
							//
							if (CheckHitKey(KEY_INPUT_SPACE) != 0) {
								startp = true;
							}
							//
						}
					}
					else {
						speed = std::clamp(speed + 1.5f / 3.6f / GetFPS(), 0.f, 20.f / 3.6f / GetFPS());
						pos.zadd(-speed);

						if (pos.z() <= -15.f) {
							easing_set(&fovs, 1.75f, 0.95f);
						}
						if (pos.z() < -20.f) {
							endp = true;
						}
					}
					//視点取得
					if (Drawparts->use_vr) {
						Drawparts->GetHMDPositionVR(&HMDpos, &HMDmat);
						cam_s.cam.campos = pos_mine + HMDpos;
						cam_s.cam.camvec = cam_s.cam.campos - HMDmat.zvec();
						cam_s.cam.camup = HMDmat.yvec();
					}
					else {
						if (!startp) {
							easing_set(&cam_s.cam.campos, (MATRIX_ref::RotX(xrad_m) * MATRIX_ref::RotY(yrad_m)).zvec() * (-15.f) + VGet(0.f, 3.f, 0.f), 0.95f);
							cam_s.cam.camvec = pos + VGet(0.f, 3.f, 0.f);
						}
						else {
							easing_set(&cam_s.cam.campos, VGet((1.f - (pos.z() / -120.f)), (1.f - (pos.z() / -120.f)) + 3.f, (1.f - (pos.z() / -120.f)) + 10.f), 0.95f);
							easing_set(&cam_s.cam.camvec, pos + VGet((1.f - (pos.z() / -120.f)), (1.f - (pos.z() / -120.f)) + 1.f, (1.f - (pos.z() / -120.f))), 0.95f);
						}
						cam_s.cam.camup = VGet(0.f, 1.f, 0.f);
					}
					//UI_buf
					{
						outScreen.SetDraw_Screen();
						{
							{
								int xp = Drawparts->disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(90)));
								int yp = Drawparts->disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(90)));
								int xa = Drawparts->disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(90)));
								int ya = Drawparts->disp_y * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(90)));
								DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
								{
									DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 0, 0), TRUE);
									font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3), GetColor(0, 255, 0), "Name     :%s", Vehicles[veh.use_id].name.c_str());
									font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 20), GetColor(0, 255, 0), "MaxSpeed :%03.0f km/h", Vehicles[veh.use_id].max_speed_limit*3.6f);
									font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 40), GetColor(0, 255, 0), "MidSpeed :%03.0f km/h", Vehicles[veh.use_id].mid_speed_limit*3.6f);
									font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 60), GetColor(0, 255, 0), "MinSpeed :%03.0f km/h", Vehicles[veh.use_id].min_speed_limit*3.6f);
									font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 80), GetColor(0, 255, 0), "Turn     :%03.0f °/s", Vehicles[veh.use_id].body_rad_limit);
									DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 255, 0), FALSE);
									font12.DrawString(xp - y_r(120), yp - y_r(60 + 15), "Spec", GetColor(0, 255, 0));
								}
							}

							{
								int xp = Drawparts->disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(180)));
								int yp = Drawparts->disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(180)));
								int xa = Drawparts->disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(180)));
								int ya = Drawparts->disp_y * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(180)));
								DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
								{
									int ys = 20 * int(Vehicles[veh.use_id].gunframe.size()) / 2 + 1;
									DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 0, 0), TRUE);
									if (Vehicles[veh.use_id].gunframe.size() == 0) {
										font18.DrawString(xp - y_r(120 - 3), yp - y_r(ys - 3), "N/A", GetColor(0, 255, 0));
									}
									else {
										for (int z = 0; z < Vehicles[veh.use_id].gunframe.size(); z++) {
											font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(ys - 3 - 20 * z), GetColor(0, 255, 0), "No.%d  :%s", z, Vehicles[veh.use_id].gunframe[z].name.c_str());
										}
									}
									DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 255, 0), FALSE);
									font12.DrawString(xp - y_r(120), yp - y_r(ys + 15), "Weapon", GetColor(0, 255, 0));
								}
							}
						}
						easing_set(&ber_r, float(Drawparts->disp_y / 4), 0.95f);
						easing_set(&rad, deg2rad(yrad_im), 0.9f);
					}
					Drawparts->Move_Player();
					//自機描画
					{
						//cam_s.cam
						{
							//far取得
							cam_s.cam.far_ = 300.f;
							//near取得
							cam_s.cam.near_ = 0.1f;
							//fov
							cam_s.cam.fov = deg2rad(Drawparts->use_vr ? 90 : fov_pc / fovs);
						}
						//
						//VRに移す
						Drawparts->draw_VR(
							[&] {
							auto tmp = GetDrawScreen();
							auto tmp_cams = cam_s;
							//*
							auto camtmp = VECTOR_ref(GetCameraPosition()) - cam_s.cam.campos;

							auto tvec = (VECTOR_ref(cam_s.cam.camvec) - cam_s.cam.campos);

							camtmp = MATRIX_ref::Vtrans(camtmp, MATRIX_ref::Axis1(
								tvec.cross(cam_s.cam.camup),
								cam_s.cam.camup,
								tvec));

							tmp_cams.cam.campos = camtmp + cam_s.cam.campos;
							tmp_cams.cam.camvec = camtmp + cam_s.cam.camvec;
							//*/
							GraphHandle::SetDraw_Screen(tmp, tmp_cams.cam.campos, tmp_cams.cam.camvec, tmp_cams.cam.camup, tmp_cams.cam.fov, tmp_cams.cam.near_, tmp_cams.cam.far_);
							{
								garage.DrawModel();
								Vehicles[veh.use_id].obj.SetMatrix(MATRIX_ref::Mtrans(pos));
								Vehicles[veh.use_id].obj.DrawModel();
								SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(255 - int(255.f * pos.z() / -10.f), 0, 255));
								outScreen.DrawGraph(0, 0, true);
								SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f * (pos.z() + 60.f) / -60.f), 0, 255));
								DrawBox(0, 0, Drawparts->disp_x, Drawparts->disp_y, GetColor(255, 255, 255), TRUE);
								SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
								//UIparts->item_draw(chara, mine, tmp_cams);
							}
						}, cam_s.cam);
					}
					//draw
					GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
					{
						if (Drawparts->use_vr) {
							Drawparts->outScreen[0].DrawRotaGraph(960, 540, 0.5f, 0, false);
						}
						else {
							Drawparts->outScreen[0].DrawGraph(0, 0, false);
						}
					}
					Drawparts->Screen_Flip(waits);
					if (endp) {
						WaitTimer(100);
						break;
					}
					if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
						break;
					}
				}
				if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
					break;
				}
				{
					MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
					bgm_title.stop();
				}
			}
			//開始
			Drawparts->reset_HMD();
			start_c2 = true;
			{
				{
					//マップ読み込み
					mapparts->set_map_pre();
					UIparts->load_window("マップモデル");			   //ロード画面
					mapparts->set_map("data/grassput.bmp", VGet(0.0f, -0.5f, 0.5f), 35000.f, 35000.f, -35000.f, -35000.f);
					//光、影
					Drawparts->Set_Light_Shadow(mapparts->map_get().mesh_maxpos(0), mapparts->map_get().mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f),
						[&] {
						mapparts->map_get().DrawModel();
						mapparts->cloud_draw();
					});
					SetGlobalAmbientLight(GetColorF(0.45f, 0.5f, 0.5f, 1.f));
					//共通
					for (auto& c : chara) {
						//キャラ選択
						size_t i = &c - &chara[0];
						if (i >= (chara.size() / 2)) {
							c.type = 1;
						}
						if (i != 0) {
							c.vehicle.use_id = GetRand(int(Vehicles.size() - 1));
						}
						float rad = deg2rad(-130);
						c.vehicle.spawn(VGet(float(-2000 + 4000 * int(i / (chara.size() / 2)))*sin(rad) + float(100 * (i % (chara.size() / 2)))*cos(rad), 1500.f, float(-2000 + 4000 * int(i / (chara.size() / 2)))*cos(rad) - float(100 * (i % (chara.size() / 2)))*sin(rad)), MATRIX_ref::RotY(deg2rad(((c.type == 0) ? 180 : 0) - 130)));
						c.set_human(Vehicles, Ammo);	//set
						c.cocks.set_(cockpit);			//コックピット
						c.se.Duplicate(se);				//se
					}
					se_alert.vol(int(float(192)*se_vol));
					se_alert2.vol(int(float(192)*se_vol));
					eye_pos_ads = VGet(0, 0.58f, 0);
					eyevec = mine.vehicle.mat.zvec() * -1.f;
					cam_s.cam.campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyevec * range;
					cam_s.Rot = ADS;
					eyevec2 = chara[0].vehicle.mat.zvec() * -1.f;
					for (auto& c : chara) {
						for (auto& t : c.vehicle.use_veh.wheelframe) {
							t.init();
						}
						c.se.cockpit.play(DX_PLAYTYPE_LOOP, TRUE);
						c.se.cockpit.vol(int(float(128)*se_vol));
						c.se.engine.play(DX_PLAYTYPE_LOOP, TRUE);
					}
					bgm_main.play(DX_PLAYTYPE_LOOP, TRUE);
					bgm_main.vol(int(float(255)*0.5f));
					bgm_win.vol(int(float(255)*0.5f));
					voice_[0].set(0.f, voice_str);
					SetMouseDispFlag(FALSE);
					SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
					timer = 60.f*5.f;
					ready_timer = 5.f;
				}
				//
				while (ProcessMessage() == 0) {
					const auto waits = GetNowHiPerformanceCount();
					Debugparts->put_way();
					//
					for (auto& c : chara) {
						c.set_alive(se_vol);
					}
					Debugparts->end_way();
					//プレイヤー操作+CPU
					{
						//スコープ
						{
							cam_s.Rot = std::clamp(cam_s.Rot + GetMouseWheelRotVol(), 0, 3);
							if (Drawparts->use_vr) {
								switch (cam_s.Rot) {
								case 2:
									fovs_p = 1.f;
									break;
								case 3:
									fovs_p = 2.f;
									break;
								default:
									cam_s.Rot = ADS;
									fovs_p = 1.f;
									break;
								}
							}
							else {
								switch (cam_s.Rot) {
								case 1:
									range_p = 15.f;
									break;
								case 0:
									range_p = 30.f;
									break;
								case 2:
									fovs_p = 1.f;
									break;
								case 3:
									fovs_p = 2.f;
									break;
								default:
									cam_s.Rot = ADS;
									fovs_p = 1.f;
									break;
								}
							}
							easing_set(&range, range_p, 0.9f);
							easing_set(&fovs, fovs_p, 0.9f);
						}
						//見回し
						if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
							if (Drawparts->use_vr) {
								chara[1].view_yrad = 0.f;
								chara[1].view_xrad = 0.f;
							}
							else {
								mine.view_yrad = 0.f;
								mine.view_xrad = 0.f;
							}
						}
						//キー
						{
							//cpu
							for (auto& c : chara) {
								if (&c - &chara[0] >= (Drawparts->use_vr ? 1 : 1)) {
									c.cpu_doing(&chara);
								}
							}
							//VR専用
							if (Drawparts->use_vr) {
								std::for_each(mine.key.begin(), mine.key.end(), [](bool& g) {g = false; });

								auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
								if (&ptr_LEFTHAND != nullptr) {
									if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
										//メイン武器
										mine.key[0] = mine.key[0] || ((ptr_LEFTHAND.on[0] & BUTTON_TRIGGER) != 0);
										//サブ武器
										mine.key[1] = mine.key[1] || ((ptr_LEFTHAND.on[1] & BUTTON_TOPBUTTON_B) != 0);
										//ピッチ
										mine.key[2] = mine.key[2] || (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(24)));
										mine.key[3] = mine.key[3] || (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-18)));
										//ロール
										mine.key[4] = mine.key[4] || (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(20)));
										mine.key[5] = mine.key[5] || (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-20)));
										if ((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) {
											//ヨー
											mine.key[6] = mine.key[6] || (ptr_LEFTHAND.touch.x() > 0.5f);
											mine.key[7] = mine.key[7] || (ptr_LEFTHAND.touch.x() < -0.5f);
											//スロットル
											mine.key[8] = mine.key[8] || (ptr_LEFTHAND.touch.y() > 0.5f);
											mine.key[9] = mine.key[9] || (ptr_LEFTHAND.touch.y() < -0.5f);
											//ブレーキ
											if ((ptr_LEFTHAND.touch.x() >= -0.5f) && (ptr_LEFTHAND.touch.x() <= 0.5f) && (ptr_LEFTHAND.touch.y() >= -0.5f) && (ptr_LEFTHAND.touch.y() <= 0.5f)) {
												mine.key[11] = mine.key[11] || true;
											}
										}
										//脚
										mine.key[10] = false;
										//精密操作
										{
											//ピッチ
											mine.key[12] = mine.key[12] || (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(14)));
											mine.key[13] = mine.key[13] || (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-8)));
											//ロール
											mine.key[14] = mine.key[14] || (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(12)));
											mine.key[15] = mine.key[15] || (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-12)));
											if ((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) {
												//ヨー
												mine.key[16] = mine.key[16] || (ptr_LEFTHAND.touch.x() > 0.45f);
												mine.key[17] = mine.key[17] || (ptr_LEFTHAND.touch.x() < -0.45f);
											}
										}
										//
									}
								}
							}
							//通常
							else {
								k_.set(mine);
								//ロックオン外し
								if (k_.key_use_ID[12].get_key(0)) {
									UIparts->reset_lock();
								}
								//武装切り替え
								if (k_.key_use_ID[14].get_key(2)) {
									++mine.vehicle.sel_weapon;
									mine.vehicle.sel_weapon %= (mine.vehicle.Gun_.size() - 1);
								}
							}
							for (auto& c : chara) {
								if (c.death) {
									std::for_each(c.key.begin(), c.key.end(), [](bool& g) {g = false; });
								}
								//
								if (ready_timer >= 0.f || timer <= 0.f) {
									c.key[0] = false;
									c.key[1] = false;
								}
								//
								if (&c != &mine && (c.type == mine.type)) {
									if (c.vehicle.HP > 0) {
										if (c.key[0] && GetRand(100) < 10) {
											voice_[1].set(float(GetRand(170) + 30) / 10.f, voice_str);
										}
										if (c.key[1] && GetRand(100) < 10) {
											voice_[2].set(float(GetRand(170) + 30) / 10.f, voice_str);
										}
										if (c.vehicle.hitf) {
											voice_[3].set(float(GetRand(170) + 30) / 10.f, voice_str);
											c.vehicle.hitf = false;
										}
										if (c.vehicle.killf) {
											voice_[4].set(0.f, voice_str);
											c.vehicle.killf = false;
										}
										if (c.aim_cnt > 0 && GetRand(100) < 10) {
											voice_[5].set(float(GetRand(170) + 30) / 10.f, voice_str);
										}
										if (c.missile_cnt > 0 && GetRand(100) < 10) {
											voice_[6].set(float(GetRand(170) + 30) / 10.f, voice_str);
										}
										if (c.vehicle.dmgf) {
											voice_[7].set(float(GetRand(170) + 30) / 10.f, voice_str);
											c.vehicle.dmgf = false;
										}
									}
									if (c.vehicle.deathf) {
										voice_[8].set(0.f, voice_str);
										c.vehicle.deathf = false;
									}
								}
							}
							//
							for (auto&v : voice_) {
								v.timer = std::max(v.timer - 1.f / GetFPS(), 0.f);
							}
							for (auto& v : voice_str) {
								if (CheckSoundMem(v.handle->get()) != TRUE) {
									voice_str.erase(voice_str.begin() + (&v - &voice_str[0]));
									break;
								}
							}
						}
						//マウスと視点角度をリンク
						if (Drawparts->use_vr) {
							//+視点取得
							Drawparts->GetHMDPositionVR(&HMDpos, &HMDmat);
							eye_pos_ads = HMDpos + VGet(0, -0.42f, 0);
							eye_pos_ads = VGet(
								std::clamp(eye_pos_ads.x(), -0.18f, 0.18f),
								std::clamp(eye_pos_ads.y(), 0.f, 0.8f),
								std::clamp(eye_pos_ads.z(), -0.36f, 0.1f)
							);
							eyevec = HMDmat.zvec();
							//
						}
						else {
							mouse_aim(eyevec);
						}
					}
					//反映
					Debugparts->end_way();
					for (auto& c : chara) {
						//反映
						c.update(mapparts, Drawparts, &chara, start_c2);
						//サウンド
						c.se.setpos(c.vehicle.pos);
					}
					Debugparts->end_way();
					//
					start_c2 = false;
					//アラート
					{
						bool ttttt = false;
						if (mine.vehicle.speed < mine.vehicle.use_veh.min_speed_limit) {
							if (CheckSoundMem(se_alert2.get()) != TRUE) {
								se_alert2.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (mine.missile_cnt > 0) {
							if (CheckSoundMem(se_alert2.get()) != TRUE) {
								se_alert2.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (!ttttt) {
							se_alert2.stop();
						}
						ttttt = false;
						if (mine.vehicle.pos.y() <= danger_height) {
							if (CheckSoundMem(se_alert.get()) != TRUE) {
								se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (mine.aim_cnt > 0) {
							if (CheckSoundMem(se_alert.get()) != TRUE) {
								se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (!ttttt) {
							se_alert.stop();
						}
					}
					//影用意
					Drawparts->Ready_Shadow(cam_s.cam.campos, shadow_draw, VGet(100.f, 100.f, 100.f), VGet(1000.f, 1000.f, 1000.f));
					//VR更新
					Drawparts->Move_Player();
					//描画用意
					mapparts->sea_draw_set();
					//描画
					{
						//自機描画
						{
							auto& veh = mine.vehicle;
							//cam_s.cam
							{
								//campos,camvec,camup取得
								if (cam_s.Rot >= ADS) {
									cam_easy.camvec -= cam_easy.campos;
									cam_easy.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
									cam_easy.campos.y(std::max(cam_easy.campos.y(), 5.f));
									if (Drawparts->use_vr) {
										cam_easy.camvec = cam_easy.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
										cam_easy.camup = MATRIX_ref::Vtrans(HMDmat.yvec(), veh.mat);//veh.mat.yvec();
									}
									else {
										if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
											easing_set(&cam_easy.camvec, MATRIX_ref::Vtrans(eyevec, veh.mat)*-1.f, 0.75f);
											cam_easy.camvec += cam_easy.campos;

											cam_easy.camup = veh.mat.yvec();
										}
										else {
											eyevec = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
											cam_easy.camvec = cam_easy.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
											cam_easy.camup = veh.mat.yvec();
										}
									}
								}
								else {
									cam_easy.campos -= cam_easy.camvec;

									cam_easy.camvec = veh.pos + veh.mat.yvec() * (6.f);
									cam_easy.camvec.y(std::max(cam_easy.camvec.y(), 5.f));

									if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {

										easing_set(&cam_easy.campos, eyevec * range, 0.9f);
										cam_easy.campos += cam_easy.camvec;
										cam_easy.campos.y(std::max(cam_easy.campos.y(), 0.f));
										mapparts->map_col_line_nearest(cam_easy.camvec, &cam_easy.campos);

										easing_set(&cam_easy.camup, VGet(0.f, 1.f, 0.f), 0.9f);
									}
									else {
										cam_easy.camvec = veh.pos + veh.mat.yvec() * (6.f);
										cam_easy.camvec.y(std::max(cam_easy.camvec.y(), 5.f));

										eyevec = (cam_easy.camvec - (mine.vehicle.pos + mine.vehicle.mat.zvec() * (-1000.f))).Norm();
										cam_easy.campos = cam_easy.camvec + eyevec * range;

										cam_easy.camup = veh.mat.yvec();
									}
								}
								//far取得
								cam_easy.far_ = (cam_s.Rot >= ADS) ? (1000.f) : (60.f*(range_p - 5.f));
								//near取得
								cam_easy.near_ = (cam_s.Rot >= ADS) ? (3.f) : (range_p - 5.f);
								//fov
								cam_easy.fov = deg2rad(Drawparts->use_vr ? 90 : fov_pc / fovs);

								cam_s.cam = cam_easy;
							}
							//
							Set3DSoundListenerPosAndFrontPosAndUpVec(cam_s.cam.campos.get(), cam_s.cam.camvec.get(), cam_s.cam.camup.get());
							//コックピット演算
							if (cam_s.Rot >= ADS) {
								mine.cocks.ready_(mine);
							}
							//UI
							UI_Screen.SetDraw_Screen();
							{
								UIparts->draw(mine, cam_s.Rot >= ADS, *Drawparts->get_device_hand1(), danger_height, Drawparts->use_vr);
							}
							//VRに移す
							Drawparts->draw_VR(
								[&] {
								auto tmp = GetDrawScreen();
								auto tmp_cams = cam_s;
								{
									auto camtmp = VECTOR_ref(GetCameraPosition()) - cam_s.cam.campos;
									auto tvec = (VECTOR_ref(cam_s.cam.camvec) - cam_s.cam.campos);
									camtmp = MATRIX_ref::Vtrans(camtmp, MATRIX_ref::Axis1(tvec.cross(cam_s.cam.camup), cam_s.cam.camup, tvec));
									tmp_cams.cam.campos = camtmp + cam_s.cam.campos;
									tmp_cams.cam.camvec = camtmp + cam_s.cam.camvec;
								}
								//被写体深度描画
								Hostpassparts->BUF_draw([&]() { mapparts->sky_draw(); }, ram_draw, tmp_cams.cam);
								//最終描画
								Hostpassparts->MAIN_draw();

								GraphHandle::SetDraw_Screen(tmp, tmp_cams.cam.campos, tmp_cams.cam.camvec, tmp_cams.cam.camup, tmp_cams.cam.fov, tmp_cams.cam.near_, tmp_cams.cam.far_);
								{
									Hostpassparts->get_main().DrawGraph(0, 0, false);

									SetCameraNearFar(0.01f, 2.f);
									//コックピット
									if (tmp_cams.Rot >= ADS) {
										mine.cocks.obj.DrawModel();
									}
									if (Drawparts->use_vr) {
										//UI
										SetUseZBuffer3D(FALSE);												//zbufuse
										SetWriteZBuffer3D(FALSE);											//zbufwrite
										DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, 1.8f, 0.f, UI_Screen.get(), TRUE);
										SetUseZBuffer3D(TRUE);												//zbufuse
										SetWriteZBuffer3D(TRUE);											//zbufwrite
									}
									else {
										this->UI_Screen.DrawGraph(0, 0, TRUE);
									}
									UIparts->item_draw(chara, mine, tmp_cams.Rot >= ADS, danger_height, Drawparts->use_vr);
									//
									{
										int yyy = 30;
										for (auto& v : voice_str) {
											if (CheckSoundMem(v.handle->get()) == TRUE) {
												auto wide = font18.GetDrawWidth(v.str);
												SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
												DrawBox(Drawparts->disp_x / 2 - wide / 2, yyy - 2, Drawparts->disp_x / 2 + wide / 2, yyy + 18 + 2, GetColor(0, 0, 0), TRUE);
												SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

												font18.DrawString_MID(Drawparts->disp_x / 2, yyy, v.str, GetColor(100, 150, 255));
												yyy += 24;
											}
										}
									}
									//
									{
										if ((ready_timer <= 0.f) || (ready_timer > 0.f && ((GetNowHiPerformanceCount() / 100000) % 10 > 5))) {
											font18.DrawStringFormat_MID(Drawparts->disp_x / 2, Drawparts->disp_y / 2 - Drawparts->disp_y / 6, GetColor(255, 255, 0), "%02d:%02d", int(timer) / 60, int(timer) % 60);
										}
										if (timer <= 0.f) {
											//リザルト
											UIparts->res_draw(mine, Drawparts->use_vr);
										}
									}
								}
							}, cam_s.cam);
						}
						//draw
						GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
						{
							if (Drawparts->use_vr) {
								Drawparts->outScreen[0].DrawRotaGraph(960, 540, 0.5f, 0, false);
							}
							else {
								Drawparts->outScreen[0].DrawGraph(0, 0, false);
							}
							//キー
							k_.draw();
							//デバック
							Debugparts->end_way();
							Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
						}
					}
					Drawparts->Screen_Flip(waits);
					//
					if (ready_timer <= 0.f) {
						timer -= 1.f / GetFPS();
						if (timer <= 0.f && ending_win) {
							bgm_main.stop();
							bgm_win.play(DX_PLAYTYPE_BACK, TRUE);
							ending_win = false;
						}
						if (timer <= 0.f) {
							timer = 0.f;
						}
					}
					else {
						ready_timer -= 1.f / GetFPS();

						if ((GetNowHiPerformanceCount() / 100000) % 10 == 0) {
							if (CheckSoundMem(se_timer.get()) != TRUE) {
								se_timer.play(DX_PLAYTYPE_BACK, TRUE);
							}
						}
					}
					//
					if (k_.key_use_ID[11].get_key(0)) {
						ending = false;
						break;
					}
					if (k_.key_use_ID[10].get_key(0)) {
						break;
					}
					//
				}
				//終了
				{}
				{
					ending_win = true;
					bgm_main.stop();
					bgm_win.stop();
					SetMouseDispFlag(TRUE);
					SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
					for (auto& c : chara) {
						c.se.stop();
					}
					se_alert.stop();
					se_alert2.stop();
					//解放
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						//エフェクト
						for (auto& t : c.effcs_missile) {
							t.first.handle.Dispose();
						}
						for (auto& t : c.effcs) {
							t.handle.Dispose();
						}
						for (auto& t : veh.use_veh.wheelframe) {
							t.gndsmkeffcs.handle.Dispose();
						}
						for (auto& t : veh.use_veh.wingframe) {
							t.smkeffcs.handle.Dispose();
						}
						veh.Dispose();
					}
					chara.clear();
					mapparts->delete_map();
					Drawparts->Delete_Shadow();
				}
			}
			//
		} while (ProcessMessage() == 0 && ending);
	}
};