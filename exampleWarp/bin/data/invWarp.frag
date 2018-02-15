#version 150

uniform sampler2DRect tex0;
uniform vec2 inputDimensions;
uniform vec2 upper_left;
uniform vec2 upper_right;
uniform vec2 lower_left;
uniform vec2 lower_right;
uniform vec2 powr;

in vec2 texCoordVarying;

out vec4 fragColor;

void main(){
	float destX = pow(texCoordVarying.x, powr.x);
	float destY = pow(texCoordVarying.y, powr.y);

	float sourceXUp = destX * (upper_right.x - upper_left.x) + upper_left.x;
	float sourceXDown = destX * (lower_right.x - lower_left.x) + lower_left.x;
	float sourceX = mix(sourceXUp, sourceXDown, destY);

	float sourceYLeft = destY * (lower_left.y - upper_left.y) + upper_left.y;
	float sourceYRight = destY * (lower_right.y - upper_right.y) + upper_right.y;
	float sourceY = mix(sourceYLeft, sourceYRight, destX);

	fragColor = texture(tex0,vec2(sourceX * inputDimensions.x, sourceY * inputDimensions.y));
}
