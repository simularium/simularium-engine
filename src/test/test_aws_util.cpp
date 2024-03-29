#include "test/aws/test_aws_util.h"
#include "simularium/aws/aws_util.h"
#include <aws/core/Aws.h>
#include <aws/core/utils/memory/AWSMemory.h>
#include <aws/core/utils/threading/Executor.h>
#include <aws/s3/S3Client.h>
#include <aws/transfer/TransferManager.h>
#include <iostream>

namespace aics {
namespace simularium {
    namespace test {

        TEST_F(AwsSdkTest, DownloadUsingSDK)
        {
            Aws::SDKOptions options;
            options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;

            Aws::InitAPI(options);
            {
                // snippet-start:[s3.cpp.get_object.code]
                // Assign these values before running the program
                const Aws::String bucket_name = "aics-agentviz-data";
                const Aws::String object_name = "trajectory/test.txt"; // For demo, set to a text file

                Aws::Client::ClientConfiguration config;
                config.region = "us-east-2";

                auto executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("test-pool", 10);
                Aws::Transfer::TransferManagerConfiguration tcc(executor.get());
                tcc.s3Client = std::make_shared<Aws::S3::S3Client>(config);

                auto transferManager = Aws::Transfer::TransferManager::Create(tcc);
                auto downloadHandle = transferManager->DownloadFile(bucket_name, object_name, object_name);

                downloadHandle->WaitUntilFinished();

                auto status = downloadHandle->GetStatus();
                if (status == Aws::Transfer::TransferStatus::FAILED || status == Aws::Transfer::TransferStatus::CANCELED) {
                    std::cerr << downloadHandle->GetLastError() << std::endl;
                }

                EXPECT_EQ(status, Aws::Transfer::TransferStatus::COMPLETED);
            }

            Aws::ShutdownAPI(options);
        }

        TEST_F(AwsSdkTest, DownloadUsingUtil)
        {
            aics::simularium::aws_util::Download("trajectory/test.txt", "trajectory/test.txt");
        }

    } // namespace test
} // namespace simularium
} // namespace aics
