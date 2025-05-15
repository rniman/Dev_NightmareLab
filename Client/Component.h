#pragma once

//��� ������ �𸣴� ��ü�� ������Ʈ�� �����ؾ��ϴµ� �� ������Ʈ�� ������ �پ��� �� �ֱ� ����
class Component
{
public:
	enum ComponentType {
		TIMEONOFF = 0,
	};
public:
	Component(){}
	virtual ~Component(){}

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void RegisterVariable(void* variable) = 0;

	ComponentType GetComponentType() { return m_iComponentType; }
protected:
	ComponentType m_iComponentType;
};

class ComponentTimeOnOff : public Component {
public:
	ComponentTimeOnOff();
	~ComponentTimeOnOff() {}

	virtual void Init();
	virtual void Update() override;
	virtual void RegisterVariable(void* variable) override;

	void SetLimitTime(float value);
	float GetTime() const { return m_fTime; }
	float GetSetTime() const { return m_fSetTime; }
private:
	bool* m_bSwitch;
	float m_fTime;
	float m_fSetTime;
};
