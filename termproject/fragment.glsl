#version 330 core

in vec3 FragPos;	// ��ġ��
in vec3 Normal;		//���ؽ����̴�����������ְ�

out vec4 FragColor;  // ������ü�ǻ�����

uniform vec3 objectColor; // ���� ����
uniform vec3 lightColor;  //���� ����
uniform vec3 lightPos;   //���� ��ġ


void main ()
{

	float ambientLight = 0.3;	//���� ���
	vec3 ambient = ambientLight * lightColor; 	//�ֺ� ����

	vec3 normalVector = normalize (Normal);

	vec3 lightDir = normalize(lightPos - FragPos); //ǥ�����������ġ�������ǹ����������Ѵ�

	float diffuseLight = max(dot(normalVector, lightDir), 0.0);
	vec3 diffuse = diffuseLight * lightColor;


	vec3 result =  (ambient + diffuse) * objectColor;	//���� ��ü ���� ����


	FragColor = vec4 (result, 1.0);

}
