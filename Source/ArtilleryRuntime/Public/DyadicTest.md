```
#include <iostream>
#include <bitset>

using namespace std;

int main(){
	float div = 1024;
	double coercivediv = 1024;
	for(float i = 1; i <= 1024; ++i)
	{
		float base = (i/div);
		double test = (i/coercivediv);
		//    bitset<32> a = bitset<32>( *reinterpret_cast<unsigned long*> (&base));
		for(float ip = 1; ip <= 1024; ++ip)
		{
			float basep = (ip/div);
			double testp = (ip/coercivediv); 
			float basecomp = basep*base;
			double testcomp = test*testp;
			if(testcomp != basecomp)
			{
				cout << i << "   " << ip << endl;
			}
    
		}
	}
}
```

So. this doesn't output anything. That's _REALLY_ interesting, actually. 
That means two dyadics within the range 1/1024 to 1024/1024 produce a dyadic that fits in a float when
multiplied once. This sounds stupid, but has some ramifications that are interesting for us.