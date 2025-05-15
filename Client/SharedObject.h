#pragma once
#include "ParticleObject.h"

#define sharedobject SharedObject::GetInstance()

// ��� ��ü�� ���� ������ ��ü�� �������Ѵ�.
// ��ü�鿡�� �ߺ��Ǵ� �����͸� ������ ���� �ʰ� sharedobject�� ���ؼ� �����͸� �����Ѵ�.

class SharedObject
{
private:
	SharedObject() {}
public:
	static SharedObject& GetInstance() {
		static SharedObject instance;
		return instance;
	}

	void EnableItemGetParticle(const shared_ptr<CGameObject>& object);
	void AddParticle(CParticleMesh::TYPE particleType, XMFLOAT3 pos);

	vector<shared_ptr<CParticleObject>> m_vParticleObjects;
};

