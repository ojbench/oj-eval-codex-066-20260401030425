#include "src.hpp"
#include <iostream>
#include <string>
using namespace std;
using sjtu::any_ptr;
using sjtu::make_any_ptr;

int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  // The OJ will include its own tests. Provide a minimal stub
  // that does nothing but keeps a valid executable.
  // To assist manual local checks, we also allow a small interactive mode:
  // If input consists of: type int and a value, we wrap and unwrap once.
  string cmd;
  if(!(cin>>cmd)) return 0;
  if(cmd=="int"){
    long long x;cin>>x;
    any_ptr a = make_any_ptr((int)x);
    cout<<a.unwrap<int>()<<"\n";
  }else if(cmd=="string"){
    string s;cin>>ws; getline(cin,s);
    any_ptr a = make_any_ptr(s);
    cout<<a.unwrap<string>()<<"\n";
  }else{
    // no-op for unknown commands
  }
  return 0;
}
