#pragma once

//リサイズ
#define x_r(p1 , p2x) (int(p1) * p2x / deskx)
#define y_r(p1 , p2y) (int(p1) * p2y / desky)

class UI : Mainclass {
private:
	GraphHandle circle;
	GraphHandle aim;
	GraphHandle scope;
	GraphHandle lock;
	GraphHandle hit;
	GraphHandle CamScreen;
	GraphHandle HP_per;
	float ber = 0;
	GraphHandle HP_ber;
	GraphHandle bufScreen;
	//font
	FontHandle font36;
	FontHandle font18;
	FontHandle font12;
	//空描画
	MV1 garage;
	MV1 sky;
	MV1 sea;
	GraphHandle SkyScreen;
	//
	int out_disp_x = deskx;
	int out_disp_y = desky;
	int disp_x = deskx;
	int disp_y = desky;
	bool use_vr = false;

	VECTOR_ref aimpos;
	VECTOR_ref aimpos_2;
	VECTOR_ref altpos;
	VECTOR_ref spdpos;


	float rad_tt = 0.f;
public:
	UI(const bool& use_vr_) {
		out_disp_x = deskx;
		out_disp_y = desky;
		use_vr = use_vr_;

		lock = GraphHandle::Load("data/UI/battle_lock.bmp");
		hit = GraphHandle::Load("data/UI/battle_hit.bmp");
		circle = GraphHandle::Load("data/UI/battle_circle.bmp");
		aim = GraphHandle::Load("data/UI/battle_aim.bmp");
		scope = GraphHandle::Load("data/UI/battle_scope.png");
		HP_per = GraphHandle::Load("data/UI/battle_hp_bar_max.bmp");
		HP_ber = GraphHandle::Load("data/UI/battle_hp_bar.bmp");
		CamScreen = GraphHandle::Make(240, 240, true);
		bufScreen = GraphHandle::Make(out_disp_x, out_disp_y, true);

		font36 = FontHandle::Create(y_r(36, out_disp_y), DX_FONTTYPE_EDGE);
		font18 = FontHandle::Create(y_r(18, out_disp_y), DX_FONTTYPE_EDGE);
		font12 = FontHandle::Create(y_r(12, out_disp_y), DX_FONTTYPE_EDGE);
		MV1::Load("data/model/garage/model.mv1", &garage, false);
		MV1::Load("data/model/sky/model.mv1", &sky, false);
		MV1::Load("data/model/sea/model.mv1", &sea, true);
		SkyScreen = GraphHandle::Make(out_disp_x, out_disp_y);
	}
	~UI() {
	}
	bool select_window(Mainclass::Chara* chara, std::vector<Mainclass::Vehcs>* vehcs) {
		if (0) {
			VECTOR_ref campos = VGet(0.f, 0.f, -15.f);
			VECTOR_ref camaim = VGet(0.f, 3.f, 0.f);
			uint8_t upct = 0, dnct = 0, rtct = 0, ltct = 0;
			float fov = deg2rad(90 / 2);

			chara->vehicle.use_id %= (*vehcs).size(); //飛行機
			chara->vehicle.camo_sel = std::min(chara->vehicle.camo_sel, (*vehcs)[chara->vehicle.use_id].camog.size() - 1);

			float speed = 0.f;
			VECTOR_ref pos;

			bool endp = false;
			bool startp = false;
			float rad = 0.f;
			float xrad_m = 0.f;
			float yrad_m = 0.f;
			float rad_i = 0.f;
			float yrad_im = 0.f, xrad_im = 0.f;
			int m_x = 0, m_y = 0;
			float ber_r = 0.f;
			GetMousePoint(&m_x, &m_y);

			while (ProcessMessage() == 0) {
				const auto waits = GetNowHiPerformanceCount();

				if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
					break;
				}

				if (!startp) {
					int x, y;
					GetMousePoint(&x, &y);

					yrad_im = std::clamp(yrad_im + float(m_x - x) / 5.f, -120.f, -30.f);
					xrad_im = std::clamp(xrad_im + float(m_y - y), -45.f, 45.f);

					easing_set(&yrad_m, deg2rad(yrad_im), 0.9f);
					easing_set(&xrad_m, deg2rad(xrad_im), 0.9f);
					GetMousePoint(&m_x, &m_y);

					upct = std::clamp<uint8_t>(upct + 1, 0, ((CheckHitKey(KEY_INPUT_D) != 0) ? 2 : 0));
					dnct = std::clamp<uint8_t>(dnct + 1, 0, ((CheckHitKey(KEY_INPUT_A) != 0) ? 2 : 0));
					ltct = std::clamp<uint8_t>(ltct + 1, 0, ((CheckHitKey(KEY_INPUT_S) != 0) ? 2 : 0));
					rtct = std::clamp<uint8_t>(rtct + 1, 0, ((CheckHitKey(KEY_INPUT_W) != 0) ? 2 : 0));
				}
				else {
					upct = 0;
					dnct = 0;
					ltct = 0;
					rtct = 0;
				}

				easing_set(&ber_r, float(out_disp_y / 4), 0.95f);

				auto& veh = chara->vehicle;
				{
					if (CheckHitKey(KEY_INPUT_SPACE) != 0 || speed != 0.f) {
						speed = std::clamp(speed + 2.5f / 3.6f / GetFPS(), 0.f, 200.f / 3.6f / GetFPS());
						pos.zadd(-speed);
						startp = true;
					}
					auto old = veh.camo_sel;
					if (!startp) {
						VECTOR_ref campos_t = (MATRIX_ref::RotY(yrad_m) * MATRIX_ref::RotX(xrad_m)).zvec() * (-30.f) + VGet(0.f, 3.f, 0.f);
						easing_set(&campos, campos_t + VGet(float(-200 + GetRand(400)) / 100.f, float(-200 + GetRand(400)) / 100.f, float(-200 + GetRand(400)) / 100.f), 0.95f);

						camaim = pos + VGet(0.f, 3.f, 0.f);
						if (upct == 1) {
							++veh.use_id %= (*vehcs).size();
							veh.camo_sel = std::min(veh.camo_sel, (*vehcs)[veh.use_id].camog.size() - 1);
						}
						if (dnct == 1) {
							if (veh.use_id == 0) {
								veh.use_id = (*vehcs).size() - 1;
							}
							else {
								--veh.use_id;
							}
							veh.camo_sel = std::min(veh.camo_sel, (*vehcs)[veh.use_id].camog.size() - 1);
						}
						if (ltct == 1) {
							++veh.camo_sel %= (*vehcs)[veh.use_id].camog.size();
						}
						if (rtct == 1) {
							if (veh.camo_sel == 0) {
								veh.camo_sel = (*vehcs)[veh.use_id].camog.size() - 1;
							}
							else {
								--veh.camo_sel;
							}
						}
					}
					else {
						easing_set(&campos,
							VGet(
							(float(-25 + GetRand(50)) / 100.f) * (1.f - (pos.z() / -120.f)),
								(float(-25 + GetRand(50)) / 100.f) * (1.f - (pos.z() / -120.f)),
								(float(-25 + GetRand(50)) / 100.f) * (1.f - (pos.z() / -120.f)) + 15.f),
							0.95f);
						easing_set(&camaim, pos + VGet((float(-200 + GetRand(400)) / 100.f) * (1.f - (pos.z() / -120.f)), (float(-200 + GetRand(400)) / 100.f) * (1.f - (pos.z() / -120.f)) + 3.f, (float(-200 + GetRand(400)) / 100.f) * (1.f - (pos.z() / -120.f))), 0.95f);
					}
					if ((*vehcs)[veh.use_id].camog.size() > 0) {
						SetDrawScreen(CamScreen.get());
						DrawExtendGraph(0, 0, 240, 240, (*vehcs)[veh.use_id].camog[veh.camo_sel], TRUE);
						MV1SetTextureGraphHandle((*vehcs)[veh.use_id].obj.get(), (*vehcs)[veh.use_id].camo_tex, (*vehcs)[veh.use_id].camog[veh.camo_sel], TRUE);
					}
					GraphFilter(CamScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 2400);
					if (veh.camo_sel != old) {
						if (std::abs(int(int(veh.camo_sel) - old)) == 1) {
							rad_i += 360 * int(int(veh.camo_sel) - old) / int((*vehcs)[veh.use_id].camog.size());
						}
						else {
							if (veh.camo_sel == 0) {
								rad_i += 360 / int((*vehcs)[veh.use_id].camog.size());
							}
							else {
								rad_i -= 360 / int((*vehcs)[veh.use_id].camog.size());
							}
						}
					}
					bufScreen.SetDraw_Screen();
					{
						int xp = out_disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad));
						int yp = out_disp_y * 2 / 3 + int(ber_r * cos(rad));
						int xa = out_disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150, out_disp_y)) * sin(rad));
						int ya = out_disp_y * 2 / 3 + int((ber_r - y_r(150, out_disp_y)) * cos(rad));
						DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
						CamScreen.DrawExtendGraph(xp - y_r(60, out_disp_y), yp - y_r(60, out_disp_y), xp + y_r(60, out_disp_y), yp + y_r(60, out_disp_y), true);
						DrawBox(xp - y_r(60, out_disp_y), yp - y_r(60, out_disp_y), xp + y_r(60, out_disp_y), yp + y_r(60, out_disp_y), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(60, out_disp_y), yp - y_r(60 + 15, out_disp_y), "Camo", GetColor(0, 255, 0));
					}

					{
						int xp = out_disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(90)));
						int yp = out_disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(90)));
						int xa = out_disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150, out_disp_y)) * sin(rad + deg2rad(90)));
						int ya = out_disp_y * 2 / 3 + int((ber_r - y_r(150, out_disp_y)) * cos(rad + deg2rad(90)));
						DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);

						DrawBox(xp - y_r(120, out_disp_y), yp - y_r(60, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(60, out_disp_y), GetColor(0, 0, 0), TRUE);

						font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3, out_disp_y), GetColor(0, 255, 0), "Name     :%s", (*vehcs)[veh.use_id].name.c_str());
						font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 20, out_disp_y), GetColor(0, 255, 0), "MaxSpeed :%03.0f km/h", (*vehcs)[veh.use_id].max_speed_limit*3.6f);
						font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 40, out_disp_y), GetColor(0, 255, 0), "MidSpeed :%03.0f km/h", (*vehcs)[veh.use_id].mid_speed_limit*3.6f);
						font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 60, out_disp_y), GetColor(0, 255, 0), "MinSpeed :%03.0f km/h", (*vehcs)[veh.use_id].min_speed_limit*3.6f);
						font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 80, out_disp_y), GetColor(0, 255, 0), "Turn     :%03.0f °/s", (*vehcs)[veh.use_id].body_rad_limit);

						DrawBox(xp - y_r(120, out_disp_y), yp - y_r(60, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(60, out_disp_y), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(120, out_disp_y), yp - y_r(60 + 15, out_disp_y), "Spec", GetColor(0, 255, 0));
					}

					{
						int xp = out_disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(180)));
						int yp = out_disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(180)));
						int xa = out_disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150, out_disp_y)) * sin(rad + deg2rad(180)));
						int ya = out_disp_y * 2 / 3 + int((ber_r - y_r(150, out_disp_y)) * cos(rad + deg2rad(180)));
						DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);

						int ys = 20 * int((*vehcs)[veh.use_id].gunframe.size()) / 2 + 1;

						DrawBox(xp - y_r(120, out_disp_y), yp - y_r(ys, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(ys, out_disp_y), GetColor(0, 0, 0), TRUE);

						if ((*vehcs)[veh.use_id].gunframe.size() == 0) {
							font18.DrawString(xp - y_r(120 - 3, out_disp_y), yp - y_r(ys - 3, out_disp_y), "N/A", GetColor(0, 255, 0));
						}
						else {
							for (int z = 0; z < (*vehcs)[veh.use_id].gunframe.size(); z++) {
								font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(ys - 3 - 20 * z, out_disp_y), GetColor(0, 255, 0), "No.%d  :%s", z, (*vehcs)[veh.use_id].gunframe[z].name.c_str());
							}
						}
						DrawBox(xp - y_r(120, out_disp_y), yp - y_r(ys, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(ys, out_disp_y), GetColor(0, 255, 0), FALSE);
						font12.DrawString(xp - y_r(120, out_disp_y), yp - y_r(ys + 15, out_disp_y), "Weapon", GetColor(0, 255, 0));
					}


					SkyScreen.SetDraw_Screen(campos - camaim, VGet(0, 0, 0), VGet(0.f, 1.f, 0.f), fov, 1000.0f, 5000.0f);
					{
						SetFogEnable(FALSE);
						SetUseLighting(FALSE);
						sky.DrawModel();
						SetUseLighting(TRUE);
						SetFogEnable(TRUE);
					}

					easing_set(&rad, deg2rad(rad_i + yrad_im), 0.9f);

					GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, campos, camaim, VGet(0.f, 1.f, 0.f), fov, 3.0f, 150.f);
					{
						SkyScreen.DrawGraph(0, 0, false);
						(*vehcs)[veh.use_id].obj.SetMatrix(MATRIX_ref::Mtrans(pos));
						(*vehcs)[veh.use_id].obj.DrawModel();
					}
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(255 - int(255.f * pos.z() / -10.f), 0, 255));
					bufScreen.DrawGraph(0, 0, true);
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f * (pos.z() + 60.f) / -60.f), 0, 255));
					DrawBox(0, 0, out_disp_x, out_disp_y, GetColor(255, 255, 255), TRUE);
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					if (pos.z() <= -30.f && pos.z() > -60.f) {
						easing_set(&fov, deg2rad(90 / 2) / 2.f, 0.95f);
					}
					if (pos.z() <= -60.f) {
						easing_set(&fov, deg2rad(90 / 2) / 4.f, 0.95f);
					}

					if (pos.z() < -120.f) {
						endp = true;
					}
				}
				ScreenFlip();
				if (GetWaitVSyncFlag() == FALSE) {
					while (GetNowHiPerformanceCount() - waits < 1000000.0f / 60.f) {
					}
				}
				if (endp) {
					WaitTimer(100);
					break;
				}
			}

		}
		else {
			chara->vehicle.use_id = 0; //飛行機
			chara->vehicle.camo_sel = 0;
		}
		return (CheckHitKey(KEY_INPUT_ESCAPE) == 0);
	}

	void load_window(const char* mes) {
		SetUseASyncLoadFlag(FALSE);
		float bar = 0.f, cnt = 0.f;
		auto all = GetASyncLoadNum();
		while (ProcessMessage() == 0) {
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			{
				font18.DrawStringFormat(0, out_disp_y - y_r(70, out_disp_y), GetColor(0, 255, 0), " loading... : %04d/%04d  ", all - GetASyncLoadNum(), all);
				font12.DrawStringFormat(out_disp_x - font12.GetDrawWidthFormat("%s 読み込み中 ", mes), out_disp_y - y_r(70, out_disp_y), GetColor(0, 255, 0), "%s 読み込み中 ", mes);
				DrawBox(0, out_disp_y - y_r(50, out_disp_y), int(float(out_disp_x) * bar / float(all)), out_disp_y - y_r(40, out_disp_y), GetColor(0, 255, 0), TRUE);
				easing_set(&bar, float(all - GetASyncLoadNum()), 0.95f);
			}
			ScreenFlip();
			if (GetASyncLoadNum() == 0) {
				cnt += 1.f / GetFPS();
				if (cnt > 1 && bar > float(all - GetASyncLoadNum()) * 0.95f) {
					break;
				}
			}
		}
	}

	void draw(
		std::vector<Mainclass::Chara>& charas,
		GraphHandle& MAIN_Screen,
		Mainclass::CAMS& cam_s,
		Mainclass::cockpits & cocks,
		const DXDraw::system_VR& vr_sys,
		const Mainclass::Chara& chara,
		const char& overrider = -1
	) {
		int xs = 0, xp = 0, ys = 0, yp = 0;
		//照準座標取得
		{
			auto scr = GetDrawScreen();
			MAIN_Screen.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, 0.01f, 5000.0f);
			{
				VECTOR_ref aimp = (VECTOR_ref)(chara.vehicle.pos) + chara.vehicle.mat.zvec() * (-1000.f);
				//mapparts->map_col_line_nearest(chara.vehicle.pos, &aimp);				//地形
				aimpos = ConvWorldPosToScreenPos(aimp.get());
				for (auto& c : charas) {
					c.winpos = ConvWorldPosToScreenPos(c.vehicle.pos.get());
				}
				if (cam_s.Rot >= ADS) {
					altpos = ConvWorldPosToScreenPos((cocks.obj.frame(cocks.alt_100_f.first) - (cocks.obj.frame(cocks.alt_100_2_f.first) - cocks.obj.frame(cocks.alt_100_f.first)).Norm()*0.05f).get());
					spdpos = ConvWorldPosToScreenPos((cocks.obj.frame(cocks.speed_f.first) - (cocks.obj.frame(cocks.speed2_f.first) - cocks.obj.frame(cocks.speed_f.first)).Norm()*0.05f).get());

					auto& veh = chara.vehicle;
					aimpos_2 = ConvWorldPosToScreenPos((veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(-0.15f, 0.58f, -1.f), veh.mat)).get());
				}
			}
			SetDrawScreen(scr);
		}
		//オーバーライド
		MAIN_Screen.GetSize(&disp_x, &disp_y);
		//照準
		{
			int siz = int(64.f);
			if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
				circle.DrawExtendGraph(int(aimpos.x()) - y_r(siz, out_disp_y), int(aimpos.y()) - y_r(siz, out_disp_y), int(aimpos.x()) + y_r(siz, out_disp_y), int(aimpos.y()) + y_r(siz, out_disp_y), TRUE);

				VECTOR_ref tmp = cam_s.cam.camup.Norm();
				VECTOR_ref tmp_t = VGet(0, 1.f, 0);

				float tmp_cos = asinf((tmp.cross((cam_s.cam.camvec - cam_s.cam.campos).Norm())).dot(tmp_t));
				
				float rad_t = tmp_cos;
				rad_t = tmp_cos;
				if (tmp.dot(tmp_t) < 0.f) {
					rad_t = DX_PI_F - tmp_cos;
				}

				DrawRotaGraph(int(aimpos.x()), int(aimpos.y()), y_r(siz, out_disp_y) / 100.f, rad_t, aim.get(), TRUE);

				font18.DrawStringFormat(int(aimpos.x()), int(aimpos.y()), GetColor(255, 0, 0), "%d", int(rad2deg(rad_t)));
			}
		}
		//
		FontHandle* font = (!(use_vr && overrider == -1)) ? &font18 : &font36;
		{
			{
				//弾薬
				{
					if (!(use_vr && overrider == -1)) {
						xs = x_r(200, out_disp_x);
						xp = x_r(20, out_disp_x);
						ys = y_r(18, out_disp_y);
						yp = disp_y - y_r(20, out_disp_y) - int(chara.vehicle.Gun_.size()) * (ys * 2 + y_r(7, out_disp_y));
					}
					else {
						xs = x_r(200, out_disp_x);
						xp = disp_x / 2 - x_r(20, out_disp_x) - xs;
						ys = y_r(36, out_disp_y);
						yp = disp_y / 2 + disp_y / 6 + y_r(20, out_disp_y)// - int(chara.vehicle.Gun_.size()) * (ys * 2 + y_r(3, out_disp_y))
							;
					}
					int i = 0;
					for (auto& veh : chara.vehicle.Gun_) {
						if (veh.loadcnt != 0.f) {
							DrawBox(
								xp,
								yp + ys * 2 / 3,
								xp + x_r(200 - int(200.f * veh.loadcnt / veh.gun_info.load_time), out_disp_x),
								yp + ys,
								GetColor(255, 0, 0), TRUE);
						}
						else {
							DrawBox(
								xp,
								yp + ys * 2 / 3,
								xp + xs,
								yp + ys,
								GetColor(0, 255, 0), TRUE);
						}

						if (veh.rounds != 0.f) {
							DrawBox(
								xp,
								yp + ys * 2 - y_r(2, out_disp_y),
								xp + x_r(int(200.f * veh.rounds / veh.gun_info.rounds), out_disp_x),
								yp + ys * 2 + y_r(2, out_disp_y),
								GetColor(255, 192, 0), TRUE);
						}

						font->DrawString(xp, yp, veh.bullet[veh.usebullet].spec.name_a, GetColor(255, 255, 255));
						font->DrawStringFormat(xp + xs - font->GetDrawWidthFormat("%04d / %04d", veh.rounds, veh.gun_info.rounds), yp + ys + y_r(2, out_disp_y), GetColor(255, 255, 255), "%04d / %04d", veh.rounds, veh.gun_info.rounds);
						i++;

						xp += x_r(-(30 / int(chara.vehicle.Gun_.size())), out_disp_x);
						yp += (ys * 2 + y_r(4, out_disp_y));
					}
				}
				//飛行機モード用
				{
					auto& veh = chara.vehicle;
					//速度計
					{
						if (cam_s.Rot >= ADS) {
							if (spdpos.z() >= 0.f && spdpos.z() <= 1.f) {
								xp = (int)(spdpos.x());
								yp = (int)(spdpos.y());
								font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 0, GetColor(0, 255, 0), "%4.0f km/h", veh.speed * 3.6f);

								if (veh.over_heat) {
									font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 0, 0), "POWER %03.0f%%", veh.accel);
									if ((GetNowHiPerformanceCount() / 100000) % 5 <= 2) {
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 0, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
									}
								}
								else {
									if (veh.accel >= 100.f) {
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 255, 0), "POWER %03.0f%%", veh.accel);
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 255, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
									}
									else {
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(0, 255, 0), "POWER %03.0f%%", veh.accel);
									}
								}
							}
						}
						else {
							if (!(use_vr && overrider == -1)) {
								xp = disp_x / 3;
								yp = disp_y / 2;
							}
							else {
								xp = disp_x / 2 - disp_y / 6 + y_r(240 / 2, out_disp_y);
								yp = disp_y / 2;
							}
							font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 0, GetColor(255, 255, 255), "%4.0f km/h", veh.speed * 3.6f);

							if (veh.over_heat) {
								font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 0, 0), "POWER %03.0f%%", veh.accel);
								if ((GetNowHiPerformanceCount() / 100000) % 5 <= 2) {
									font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 0, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
								}
							}
							else {
								if (veh.accel >= 100.f) {
									font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 255, 0), "POWER %03.0f%%", veh.accel);
									font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 255, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
								}
								else {
									font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 255, 255), "POWER %03.0f%%", veh.accel);
								}
							}
						}
					}
					//高度計
					{
						//
						if (cam_s.Rot >= ADS) {
							if (altpos.z() >= 0.f && altpos.z() <= 1.f) {
								xp = (int)(altpos.x());
								yp = (int)(altpos.y());
								font->DrawStringFormat(xp, yp, GetColor(0, 255, 0), " %4d m", int(veh.pos.y()));
							}
						}
						else {
							//
							if (!(use_vr && overrider == -1)) {
								xp = disp_x * 2 / 3;
								yp = disp_y / 2;
							}
							else {
								xp = disp_x / 2 + (disp_y / 3 - y_r(240, out_disp_y)) / 2;
								yp = disp_y / 2;
							}
							font->DrawStringFormat(xp, yp, GetColor(255, 255, 255), " %4d m", int(veh.pos.y()));
						}
					}
					//アラート
					{
						//
						if (cam_s.Rot >= ADS) {
							if (aimpos_2.z() >= 0.f && aimpos_2.z() <= 1.f) {
								xp = (int)(aimpos_2.x());
								yp = (int)(aimpos_2.y());
								int ccc = 0;
								if (veh.speed < veh.use_veh.min_speed_limit) {
									if ((GetNowHiPerformanceCount() / 100000) % 4 <= 2) {
										font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "STALL", GetColor(255, 0, 0));
									}
									ccc++;
								}
								if (veh.pos.y() <= 30.f) {
									font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "GPWS", GetColor(255, 255, 0));
									ccc++;
								}
								if (chara.p_anime_geardown.second > 0.5f) {
									font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "GEAR DOWN", GetColor(255, 255, 0));
									ccc++;
								}
							}
						}
						else {
							if (!(use_vr && overrider == -1)) {
								xp = disp_x / 2;
								yp = disp_y / 3;
							}
							else {
								xp = disp_x / 2;
								yp = disp_y / 2 - disp_y / 6 + y_r(60, out_disp_y);
							}
							int ccc = 0;
							if (veh.speed < veh.use_veh.min_speed_limit) {
								font->DrawString_MID(xp, yp + y_r(36, out_disp_y) * ccc, "STALL", GetColor(255, 0, 0));
								ccc++;
							}
							if (veh.pos.y() <= 30.f) {
								font->DrawString_MID(xp, yp + y_r(36, out_disp_y) * ccc, "GPWS", GetColor(255, 255, 0));
								ccc++;
							}
							if (chara.p_anime_geardown.second > 0.5f) {
								font->DrawString_MID(xp, yp + y_r(36, out_disp_y) * ccc, "GEAR DOWN", GetColor(255, 255, 0));
								ccc++;
							}
						}

					}
					//HP
					{
						if (!(use_vr && overrider == -1)) {
							xs = x_r(200, out_disp_x);
							xp = disp_x - x_r(20, out_disp_x) - xs;

							xp = disp_x - x_r(20 + 30, out_disp_x) - xs;

							ys = y_r(42, out_disp_y);
							yp = disp_y - y_r(20, out_disp_y) - ys;
						}
						else {
							xs = x_r(200, out_disp_x);
							xp = disp_x / 2 + x_r(20, out_disp_x);
							ys = y_r(36, out_disp_y);
							yp = disp_y / 2 + disp_y / 6 + y_r(20, out_disp_y) - ys;
						}

						//
						{
							auto per = 255;
							DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + int(ber), yp + ys / 2 + ys * 2 / 3, GetColor(per, 0, 0), TRUE);
							easing_set(&ber, float(xs * int(veh.HP) / int(veh.use_veh.HP)), 0.975f);
							DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs * int(veh.HP) / int(veh.use_veh.HP), yp + ys / 2 + ys * 2 / 3, GetColor(0, per, 0), TRUE);
							SetDrawBright(per, per, per);
							font->DrawStringFormat(
								xp,
								yp + ys + (ys * 2 / 3 - y_r(12, out_disp_y)) / 2,
								GetColor(255, 255, 255), "%d / %d", int(veh.HP), int(veh.use_veh.HP));

							font->DrawStringFormat(
								xp + (xs - font->GetDrawWidthFormat("%s", veh.use_veh.name.c_str())),
								yp + (ys * 2 / 3 - y_r(12, out_disp_y)) / 2,
								GetColor(255, 255, 255), "%s", veh.use_veh.name.c_str());

							SetDrawBright(255, 255, 255);
							yp += ys;
							xp += x_r(30, out_disp_x);
						}
					}
				}
				//
			}
		}
		//VR用オプション
		if ((use_vr && overrider == -1)) {
			const float vr_sys_yvec_y = vr_sys.yvec.y();
			const float vr_sys_yvec_x = vr_sys.yvec.x();
			const float vr_sys_touch_y = ((vr_sys.on[1] & BUTTON_TOUCHPAD) != 0) ? vr_sys.touch.y() : 0.f;
			const float vr_sys_touch_x = ((vr_sys.on[1] & BUTTON_TOUCHPAD) != 0) ? vr_sys.touch.x() : 0.f;
			//
			{
				//ピッチ、ロール
				{
					int size = y_r(10, disp_y);
					ys = disp_y / 3 - y_r(240, out_disp_y);
					xp = disp_x / 2 + ys / 2;
					yp = disp_y / 2 + ys / 2;

					int y_pos = int(float(size) * std::clamp(vr_sys_yvec_y / sin(deg2rad(20)), -2.f, 2.f));//ピッチ
					int x_pos = int(float(size) * std::clamp(vr_sys_yvec_x / sin(deg2rad(20)), -2.f, 2.f));//ロール

					for (int i = 0; i < 2; i++) {
						DrawCircle(xp, yp, size * 2, i ? GetColor(0, 255, 100) : GetColor(0, 0, 0), FALSE, 1 + i);
						DrawCircle(xp, yp, size, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), FALSE, 1 + i);
						DrawLine(xp - size / 2, yp, xp - size / 4, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DrawLine(xp, yp - size / 2, xp, yp - size / 4, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DrawLine(xp + size / 2, yp, xp + size / 4, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DrawLine(xp, yp + size / 2, xp, yp + size / 4, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
					}

					DrawCircle(xp + x_pos, yp + y_pos, size / 5, GetColor(255, 100, 50), FALSE, 2);
				}
				//ヨー
				{
					xs = disp_y / 5 - y_r(144, out_disp_y);
					xp = disp_x / 2 - xs / 2;
					yp = disp_y / 2 + disp_y / 6 - y_r(220 / 2, out_disp_y);
					int z_pos = int(float(xs / 4) * std::clamp(vr_sys_touch_x / 0.5f, -1.5f, 1.5f));//ヨー

					for (int i = 0; i < 2; i++) {
						DXDraw::Line2D(xp + xs / 2 - xs / 3, yp, xp + xs / 2 + xs / 3, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DXDraw::Line2D(xp + xs / 4, yp - 5, xp + xs / 4, yp + 5, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DXDraw::Line2D(xp + xs * 3 / 4, yp - 5, xp + xs * 3 / 4, yp + 5, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
					}
					DXDraw::Line2D(xp + xs / 2 + z_pos, yp - 5, xp + xs / 2 + z_pos, yp + 5, GetColor(255, 100, 50), 2);
				}
			}

		}
		{
			//ピッチ
			/*
			{
				int ys = disp_y / 3 - y_r(240, out_disp_y);
				int xp = disp_x / 2 - ys / 2;
				int yp = disp_y / 2 - ys / 2;
				int y_pos = int(float(ys / 4) * std::clamp(vr_sys_touch_y / 0.5f, -2.f, 2.f));

				DXDraw::Line2D(xp, yp, xp, yp + ys, GetColor(0, 0, 0), 5);
				DXDraw::Line2D(xp, yp + ys / 2 - (ys / 4), xp, yp + ys / 2 + (ys / 4), GetColor(255, 255, 255), 2);
				DXDraw::Line2D(xp, yp, xp, yp + ys / 2 - (ys / 4), GetColor(255, 0, 0), 2);
				DXDraw::Line2D(xp, yp + ys / 2 + (ys / 4), xp, yp + ys, GetColor(255, 0, 0), 2);

				DXDraw::Line2D(xp - 5, yp + ys / 2 + y_pos, xp + 5, yp + ys / 2 + y_pos, GetColor(255, 255, 0), 2);
				DXDraw::Line2D(xp - 5, yp + ys / 2 - (ys / 4), xp + 5, yp + ys / 2 - (ys / 4), GetColor(0, 255, 0), 2);
				DXDraw::Line2D(xp - 5, yp + ys / 2 + (ys / 4), xp + 5, yp + ys / 2 + (ys / 4), GetColor(0, 255, 0), 2);
			}
			*/
			//速度計
			{
				if (!(use_vr && overrider == -1)) {
					xs = x_r(200, out_disp_x);
					xp = disp_x / 3;
					ys = disp_y / 4 / 3;
					yp = disp_y / 2;
				}
				else {
					xs = 0;
					xp = disp_x / 2 - (disp_y / 3 - y_r(240, out_disp_y)) / 2;
					ys = (disp_y / 3 - y_r(240, out_disp_y)) / 3;
					yp = disp_y / 2;
				}

				DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 0, 0), 2);
				DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 255, 0), 1);

				for (int i = -ys; i < ys; i += (y_r(10, out_disp_y))) {
					int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(10, out_disp_y));
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp + 10, yp + p, GetColor(0, 255, 0), 1);
					}
					else {
						break;
					}
				}

				for (int i = -ys; i < ys; i += (y_r(100, out_disp_y))) {
					int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(100, out_disp_y));
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp + 15, yp + p, GetColor(0, 255, 0), 2);
					}
					else {
						break;
					}
				}
			}
			//高度計
			{
				if (!(use_vr && overrider == -1)) {
					xs = x_r(200, out_disp_x);
					xp = disp_x * 2 / 3;
					ys = (disp_y / 4) / 3;
					yp = disp_y / 2;
				}
				else {
					xs = 0;
					xp = disp_x / 2 + (disp_y / 3 - y_r(240, out_disp_y)) / 2;
					ys = (disp_y / 3 - y_r(240, out_disp_y)) / 3;
					yp = disp_y / 2;
				}

				DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 0, 0), 2);
				DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 255, 0), 1);

				for (int i = -ys; i < ys; i += y_r(5, out_disp_y)) {
					int p = i + int(chara.vehicle.pos.y() + ys) % y_r(5, out_disp_y);
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp - 10, yp + p, GetColor(0, 255, 0), 1);
					}
					else {
						break;
					}
				}

				for (int i = -ys; i < ys; i += y_r(50, out_disp_y)) {
					int p = i + int(chara.vehicle.pos.y() + ys) % y_r(50, out_disp_y);
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp - 15, yp + p, GetColor(0, 255, 0), 2);
					}
					else {
						break;
					}
				}
			}
		}

		for (auto&c : charas) {
			if (&c != &chara) {
				int siz = int(32.f);
				if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f) {
					siz = int(32.f);
					DrawBox(int(c.winpos.x()) - y_r(siz, out_disp_y), int(c.winpos.y()) - y_r(siz, out_disp_y), int(c.winpos.x()) + y_r(siz, out_disp_y), int(c.winpos.y()) + y_r(siz, out_disp_y), GetColor(255, 0, 0), FALSE);
					siz = int(42.f);
					DrawBox(int(c.winpos.x()) - y_r(siz, out_disp_y), int(c.winpos.y()) - y_r(siz, out_disp_y), int(c.winpos.x()) + y_r(siz, out_disp_y), int(c.winpos.y()) + y_r(siz, out_disp_y), GetColor(255, 0, 0), FALSE);

					xp = int(c.winpos.x()) - y_r(siz, out_disp_y);
					yp = int(c.winpos.y()) + y_r(siz, out_disp_y);
					xs = y_r(siz * 2, out_disp_y);
					ys = y_r(8, out_disp_y);
					DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs, yp + ys / 2 + ys * 2 / 3, GetColor(255, 0, 0), TRUE);
					DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs * int(c.vehicle.HP) / int(c.vehicle.use_veh.HP), yp + ys / 2 + ys * 2 / 3, GetColor(0, 255, 0), TRUE);
					if (c.p_anime_geardown.second >= 0.5f) {
						DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs, yp + ys / 2 + ys * 2 / 3, GetColor(127, 127, 127), TRUE);
					}


					font->DrawStringFormat(xp, yp + 18, GetColor(255, 0, 0), "%d m", int((VECTOR_ref(c.vehicle.pos) - chara.vehicle.pos).size()));
					DrawLine(int(c.winpos.x()), int(c.winpos.y()), disp_x / 2, disp_y / 2, GetColor(255, 0, 0));
				}
			}
		}

		if (chara.vehicle.KILL_ID != -1) {
			font->DrawStringFormat(disp_x / 4, disp_y / 3, GetColor(255, 0, 0), "KILL : %d", chara.vehicle.KILL);
			font->DrawStringFormat(disp_x / 4, disp_y / 3 + y_r(18, out_disp_y), GetColor(255, 0, 0), "KILL ID : %d", chara.vehicle.KILL_ID);
		}
	}
};
