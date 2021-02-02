/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <iostream>

namespace ndn {
namespace examples {

class Consumer : noncopyable
{
public:

  Consumer(const std::string &fileName): m_validator(m_face)
  {
      try {
        std::cout << "OPEN File= " << fileName << std::endl;
        m_validator.load(fileName);
      }
      catch (const std::exception &e ) {
        std::cout << "Can not load File= " << fileName << ". Error: " << e.what()
          <<  std::endl;
        exit(1);
      }

  }

  void
  run()
  { 
    // Request interest
    Interest interest(Name("/example/test/randomData"));
    interest.setInterestLifetime(time::milliseconds(1000));
    interest.setMustBeFresh(true);

    m_face.expressInterest(interest,
                           std::bind(&Consumer::onData, this,  _1, _2),
                           [](const Interest&, const lp::Nack&) {},
                           std::bind(&Consumer::onTimeout, this, _1));

    std::cout << "I >> : " << interest << std::endl;

    // processEvents will block until the requested data received or timeout occurs
    m_face.processEvents();
  }

private:
  void
  onData(const Interest& interest, const Data& data)
  {
    std::cout << "D << : " << data << std::endl;

    // Validating data
    m_validator.validate(data,
                         std::bind(&Consumer::onValidated, this, _1),
                         std::bind(&Consumer::onValidationFailed, this, _1, _2));

  }

  void
  onTimeout(const Interest& interest)
  {
    std::cout << "Timeout " << interest << std::endl;
  }

  void 
  onValidated(const Data& data)
  {
    std::cout << "Validated data: " << data.getName() << std::endl;
  }

  void 
  onValidationFailed(const Data& data, const ndn::security::ValidationError& ve)
  {
    std::cerr << "Not validated data: " << data.getName()
              << ". The failure info: " << ve << std::endl;
  }

private:
  Face m_face;
  ndn::ValidatorConfig m_validator; 
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
 if (argc != 2) {
     std::cerr << "Validation file must be specified" << std::endl;
     std::cerr << "General use:" << std::endl;
     std::cerr << "  " << argv[0] << " validation_file" << std::endl;
     return 1;
  }

  ndn::examples::Consumer consumer(argv[1]);
  try {
    consumer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
