/*
 *
 * CSED 211, Fall 2022
 * DataLAb: Manipulating Bits
 * Assigned: 2022-09-07
 * Due: 2022-09-18 23:59 pm
 *
 * Namgyu Park (namgyu.park@postech.ac.kr) is the lead person for this assignment.
 *
 */

#if 0
LAB L1 INSTRUCTIONS :
#endif
/*
 *   #Homework1-1
 *   bitNor - ~(x|y) using only ~ and &
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitNor(int x, int y) {
	//to be implemented

	return ~x & ~y; //드모르간의 법칙에 의해 
}/*
 *   #Homework1-2
 *   isZero - returns 1 if x == 0, and 0 otherwise
 *   Examples: isZero(5) = 0, isZero(0) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int isZero(int x) {
	//to be implemented
	return !x; // 0일 땐 1, 0이외의 다른 수 (true)일 땐 0 출력
}/*
 *   #Homework1-3
 *   addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1,
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {

	//to be implemented
	int sum = x + y;
	int x_sign = (x >> 31) & 0x00000001; //x의 부호 
	int y_sign = (y >> 31) & 0x00000001; //y의 부호 
	int sum_sign = (sum >> 31) & 0x00000001; //sum의 부호 

	int xysame = !(x_sign^y_sign); //x와 y의 부호가 다르면 0 return 
	int xsumdiff = x_sign^sum_sign; //x와 sum의 부호가 다르면 1 return 

	return !(xysame & xsumdiff); //x와 y가 같고 x와 sum 부호가 다를 때 0 출력, 이외의 상황 (오버플로우 발생)할 때 1 출력 

}/*
 *   #Homework1-4
 *   absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {
	//to be implemented
	int sign = x >> 31; // x의 부호만 살림 
	int ans = x + sign; 
	ans = ans ^ sign; 
	return ans; //ans 값을 return 
}/*
 *   #Homework1-5
 *   logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */

int logicalShift(int x, int n) {
	//to be implemented
	int temp = ((1 << 31) >> n) << 1; // shift한 부분까지는 1, 나머지는 0인 temp 만듦 
	x = x >> n; // arithmetic shift 실행 
	return ~temp & x; //temp의 반전: shift한 부분까지는 0, 나머지는 1이므로 x의 shift한 부분은 모두 0으로 만듦(logical shift) 
	

}
