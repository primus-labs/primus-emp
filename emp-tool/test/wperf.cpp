#include "emp-tool/emp-tool.h"
#include <iostream>
using namespace std;
using namespace emp;

void test_prg()
{

  PRG prg;
  prg.reseed(&zero_block);

  for (long long length = 2; length <= 8192; length *= 2)
  {
    long long times = 1024 * 1024 * 32 / length;
    block *data = new block[length + 1];
    char *data2 = (char *)data;

    auto start = clock_start();
    for (int i = 0; i < times; ++i)
    {
      prg.random_data(data2, length * 16);
    }
    double interval = time_from(start);

    delete[] data;
    // cout << "PRG speed with block size " << length << " :\t" << (length * times) / (interval / 1e6) << " blocks/second (" << (length * times * 128) / (interval + 0.0) * 1e6 * 1e-9 << " Gbps)" << endl;

    cout << "PRG blocks: " << length << " with times: " << times << " takes: "
         << interval / 1e6 << " (s), average: "
         << (length * times) / (interval / 1e6) << " blocks/second ( " << (length * times * 128) / (interval + 0.0) * 1e6 * 1e-9 << " Gbps)" << endl;
  }
}

void test_aes_ecb_encrypt(bool use_aes_batch = false)
{
  PRG prg;
  prg.reseed(&zero_block);

  block key;
  prg.random_block(&key, 1);

  AES_KEY KEY;
  AES_set_encrypt_key(key, &KEY);

  for (int n = 2; n <= 8192; n <<= 1)
  {
    block *msg = new block[n];
    prg.random_block(msg, n);
    int times = 1024 * 1024 * 32 / n;

    auto start = clock_start();
    if (use_aes_batch && n >= AES_BATCH_SIZE)
    {
      for (int i = 0; i < times; i++)
      {
        block *pmsg = msg;
        for (int j = 0; j < n / AES_BATCH_SIZE; j++)
        {
          AES_ecb_encrypt_blks<AES_BATCH_SIZE>(pmsg, &KEY);
          pmsg += AES_BATCH_SIZE;
        }
      }
    }
    else
    {
      for (int i = 0; i < times; i++)
      {
        AES_ecb_encrypt_blks(msg, n, &KEY);
      }
    }
    double interval = time_from(start);

    delete[] msg;

    if (use_aes_batch)
      cout << "aes_encrypt(BATCH) ";
    else
      cout << "aes_encrypt ";

    cout << "blocks: " << n << " with times: " << times << " takes: "
         << interval / 1e6 << " (s), average: "
         << (n * times) / (interval / 1e6) << " blocks/second ( " << ((n * times) / (interval / 1e6)) * 128 * 1e-9 << " Gbps)" << endl;
  }
}

#include <thread>
using namespace std;
void test_thread()
{
  cout << "0" << endl;
  auto f = []()
  {
    cout << "2" << endl;
  };
  cout << "1" << endl;

  std::thread t(f);
  t.join();

  cout << "3" << endl;
}

int main()
{
  // test_thread();
  test_aes_ecb_encrypt();
  test_aes_ecb_encrypt(true);
  test_prg();

  cout << "DONE\n";
  return 0;
}