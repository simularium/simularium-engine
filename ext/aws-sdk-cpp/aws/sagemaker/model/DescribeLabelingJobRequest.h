﻿/*
* Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
*  http://aws.amazon.com/apache2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

#pragma once
#include <aws/sagemaker/SageMaker_EXPORTS.h>
#include <aws/sagemaker/SageMakerRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <utility>

namespace Aws
{
namespace SageMaker
{
namespace Model
{

  /**
   */
  class AWS_SAGEMAKER_API DescribeLabelingJobRequest : public SageMakerRequest
  {
  public:
    DescribeLabelingJobRequest();

    // Service request name is the Operation name which will send this request out,
    // each operation should has unique request name, so that we can get operation's name from this request.
    // Note: this is not true for response, multiple operations may have the same response name,
    // so we can not get operation's name from response.
    inline virtual const char* GetServiceRequestName() const override { return "DescribeLabelingJob"; }

    Aws::String SerializePayload() const override;

    Aws::Http::HeaderValueCollection GetRequestSpecificHeaders() const override;


    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline const Aws::String& GetLabelingJobName() const{ return m_labelingJobName; }

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline bool LabelingJobNameHasBeenSet() const { return m_labelingJobNameHasBeenSet; }

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline void SetLabelingJobName(const Aws::String& value) { m_labelingJobNameHasBeenSet = true; m_labelingJobName = value; }

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline void SetLabelingJobName(Aws::String&& value) { m_labelingJobNameHasBeenSet = true; m_labelingJobName = std::move(value); }

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline void SetLabelingJobName(const char* value) { m_labelingJobNameHasBeenSet = true; m_labelingJobName.assign(value); }

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline DescribeLabelingJobRequest& WithLabelingJobName(const Aws::String& value) { SetLabelingJobName(value); return *this;}

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline DescribeLabelingJobRequest& WithLabelingJobName(Aws::String&& value) { SetLabelingJobName(std::move(value)); return *this;}

    /**
     * <p>The name of the labeling job to return information for.</p>
     */
    inline DescribeLabelingJobRequest& WithLabelingJobName(const char* value) { SetLabelingJobName(value); return *this;}

  private:

    Aws::String m_labelingJobName;
    bool m_labelingJobNameHasBeenSet;
  };

} // namespace Model
} // namespace SageMaker
} // namespace Aws
