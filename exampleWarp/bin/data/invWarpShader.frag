#version 120

// this is how we receive the texture
uniform sampler2DRect tex0;

varying vec2 texCoordVarying;


uniform vec2 inputDimentions;

uniform vec2 upper_left;
uniform vec2 upper_right;
uniform vec2 lower_left;
uniform vec2 lower_right;

uniform vec2 powr;

void main()
{
	vec2 texcoord0 = texCoordVarying;
	
	float destX = pow(texcoord0.x, powr.x);
	float destY = pow(texcoord0.y, powr.y);;
	
	float sourceXUp = destX * (upper_right.x - upper_left.x) + upper_left.x;
	float sourceXDown = destX * (lower_right.x - lower_left.x) + lower_left.x;
	float sourceX = mix(sourceXUp, sourceXDown, destY);
	
	float sourceYLeft = destY * (lower_left.y - upper_left.y) + upper_left.y;
	float sourceYRight = destY * (lower_right.y - upper_right.y) + upper_right.y;
	float sourceY = mix(sourceYLeft, sourceYRight, destX);
	
	gl_FragColor = texture2DRect(tex0,vec2(sourceX * inputDimentions.x, sourceY * inputDimentions.y));
}