/*
 * Copyright 2019 Seungbin Song
 */

#ifndef PSDN_ERROR_H
#define PSDN_ERROR_H

namespace PSDN {

class PSDNErrorType : public ErrorType {
public:
  static const int WARN_TABLE_KEY_PADDING;
};

const int PSDNErrorType::WARN_TABLE_KEY_PADDING = 1018;

} // namespace PSDN

#endif
