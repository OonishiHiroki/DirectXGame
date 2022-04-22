#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>

using namespace DirectX;

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete sprite_;
	delete model_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();
	textureHandle_ = TextureManager::Load("Mario.jpg");
	sprite_ = Sprite::Create(textureHandle_, {100, 50});
	soundDataHandle_ = audio_->LoadWave("se_sad03.wav");
	//音声再生ハンドル
	// audio_->PlayWave(soundDataHandle_);
	//音声再生
	// voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);
	// 3Dモデルの生成
	model_ = Model::Create();

	//乱数シードの設定
	std::random_device seed_gen;
	//メルセンヌ・ツイスター
	std::mt19937_64 engine(seed_gen());
	//乱数範囲(回転角用)
	std::uniform_real_distribution<float> rotDist(0.0f, XM_2PI);
	//乱数範囲(座標用)
	std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
	for (size_t i = 0; i < _countof(worldTransform_); i++) {
		// X,Y,Z方向のスケーリングの設定
		worldTransform_[i].scale_ = {1.0f, 1.0f, 1.0f};
		// X,Y,Z軸周りの回転角を設定
		worldTransform_[i].rotation_ = {rotDist(engine), rotDist(engine), rotDist(engine)};
		// X,Y,Z軸周りの平行移動を設定
		worldTransform_[i].translation_ = {posDist(engine), posDist(engine), posDist(engine)};
		//ワールドトランスフォームの初期化
		worldTransform_[i].Initialize();
	}

	//カメラ視点座標を設定
	viewProjection_.eye = {0, 0, -10};

	//カメラ注視点座標を設定
	viewProjection_.target = {10, 0, 0};

	//カメラ上方向ベクトルを設定
	viewProjection_.up = {cosf(XM_PI / 4.0f), sinf(XM_PI / 4.0f), 0.0f};

	//ビュートランスフォームの初期化
	viewProjection_.Initialize();
}

void GameScene::Update() {
	//視点移動の処理
	{
		//視点の移動ベクトル
		XMFLOAT3 move = {0, 0, 0};

		//視点の移動速さ
		const float KEyespeed = 0.2f;

		//押した方向で移動ベクトルを変更
		if (input_->PushKey(DIK_W)) {
			move = {0, 0, KEyespeed};
		} else if (input_->PushKey(DIK_S)) {
			move = {0, 0, -KEyespeed};
		}

		//視点移動(ベクトルの加算)
		viewProjection_.eye.x += move.x;
		viewProjection_.eye.y += move.y;
		viewProjection_.eye.z += move.z;

		//行列の再計算
		viewProjection_.UpdateMatrix();

		//デバックの表示
		debugText_->SetPos(50, 50);
		debugText_->Printf(
		  "eye:(%f,%f,%f)",viewProjection_.eye.x,viewProjection_.eye.y,viewProjection_.eye.z);
	}

	//注視点移動処理
	{
		XMFLOAT3 move = {0, 0, 0};

		//注視点の移動速さ
		const float KTargetSpeed = 0.2f;

		//押して方法で移動ベクトルを変更
		if (input_->PushKey(DIK_LEFT)) {
			move = {-KTargetSpeed, 0, 0};
		} else if (input_->PushKey(DIK_RIGHT)) {
			move = {KTargetSpeed, 0, 0};
		}

		//注視点移動(ベクトルの加算)
		viewProjection_.target.x += move.x;
		viewProjection_.target.y += move.y;
		viewProjection_.target.z += move.z;

		//行列の再計算
		viewProjection_.UpdateMatrix();

		//デバック再表示
		debugText_->SetPos(50, 70);
		debugText_->Printf(
		  "target:(%f,%f,%f)", viewProjection_.target.x, viewProjection_.target.y,
		  viewProjection_.target.z);
	}

	//上方向回転処理
	{
		//上方向の回転速さ[ラジアン/frame]
		const float kUpRotSpeed = 0.05f;

		//押した方向で移動ベクトルを変更
		if (input_->PushKey(DIK_SPACE)) {
			viewAngle += kUpRotSpeed;
			//2πを超えたら0に戻す
			viewAngle = fmodf(viewAngle, XM_2PI);
		}

		//上方向ベクトルを計算
		viewProjection_.up = {cosf(viewAngle), sinf(viewAngle), 0.0f};

		//行列の再計算
		viewProjection_.UpdateMatrix();

		//デバック再表示
		debugText_->SetPos(50, 90);
		debugText_->Printf(
		  "up:(%f,%f,%f)", viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);
	}
	//スプライトの今の座標を取得
	XMFLOAT2 position = sprite_->GetPosition();
	//座標を{2,0}移動
	position.x += 2.0f;
	position.y += 1.0f;
	//移動した座標をスプライトに反映
	// sprite_->SetPosition(position);
	//スペースキーを押した瞬間
	if (input_->TriggerKey(DIK_SPACE)) {
		//音声停止
		audio_->StopWave(voiceHandle_);
	}
	//デバックテキストの表示
	//書式指定付き表示
	/*debugText_->SetPos(50, 50);
	debugText_->Printf("translation:(%f,%f,%f)", 10.0f, 10.0f, 10.0f);
	debugText_->SetPos(50, 70);
	debugText_->Printf("rotation:(%f,%f,%f)", XM_PI / 4.0f, XM_PI / 4.0f, 0.0f);
	debugText_->SetPos(50, 90);
	debugText_->Printf("scale:(%f,%f,%f)", 5.0f, 5.0f, 5.0f);*/
	// debugText_->Printf("year:%d", 2001);

	//変数の値をインクリメント
	value_++;
	//値を含んだ文字列
	// std::string strDebug = std::string("Value:") + std::to_string(value_);
	//デバックテキストの表示
	// debugText_->Print(strDebug, 50, 50, 1.0f);
}
void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	// 3Dモデル描画
	for (size_t i = 0; i < _countof(worldTransform_); i++) {
		model_->Draw(worldTransform_[i], viewProjection_, textureHandle_);
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>
	// sprite_->Draw();
	//  デバッグテキストの描画
	debugText_->DrawAll(commandList);
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
