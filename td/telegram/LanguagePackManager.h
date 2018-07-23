//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/net/NetQuery.h"

#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"

#include "td/actor/actor.h"
#include "td/actor/PromiseFuture.h"

#include "td/utils/Container.h"
#include "td/utils/Slice.h"
#include "td/utils/Status.h"

#include <mutex>
#include <unordered_map>
#include <utility>

namespace td {

class LanguagePackManager : public NetQueryCallback {
 public:
  explicit LanguagePackManager(ActorShared<> parent) : parent_(std::move(parent)) {
  }
  LanguagePackManager(const LanguagePackManager &) = delete;
  LanguagePackManager &operator=(const LanguagePackManager &) = delete;
  LanguagePackManager(LanguagePackManager &&) = delete;
  LanguagePackManager &operator=(LanguagePackManager &&) = delete;
  ~LanguagePackManager() override;

  static bool check_language_pack_name(Slice name);

  static bool check_language_code_name(Slice name);

  void on_language_pack_changed();

  void on_language_code_changed();

  void on_language_pack_version_changed(int32 new_version);

  void get_languages(Promise<td_api::object_ptr<td_api::languagePack>> promise);

  void get_language_pack_strings(string language_code, vector<string> keys,
                                 Promise<td_api::object_ptr<td_api::languagePackStrings>> promise);

  void on_update_language_pack(tl_object_ptr<telegram_api::langPackDifference> difference);

 private:
  struct PluralizedString;
  struct Language;
  struct LanguagePack;
  struct LanguageDatabase;

  ActorShared<> parent_;

  string language_pack_;
  string language_code_;
  LanguageDatabase *database_ = nullptr;

  static int32 manager_count_;

  static std::mutex language_database_mutex_;
  static std::unordered_map<string, std::unique_ptr<LanguageDatabase>> language_databases_;

  static Language *get_language(LanguageDatabase *database, const string &language_pack, const string &language_code);
  static Language *get_language(LanguagePack *language_pack, const string &language_code);

  static Language *add_language(LanguageDatabase *database, const string &language_pack, const string &language_code);

  static bool language_has_string_unsafe(Language *language, const string &key);
  static bool language_has_strings(Language *language, const vector<string> &keys);

  static void load_language_string_unsafe(Language *language, const string &key, string &value);
  static bool load_language_strings(LanguageDatabase *database, Language *language, const vector<string> &keys);

  static td_api::object_ptr<td_api::LanguagePackString> get_language_pack_string_object(
      const std::pair<string, string> &str);
  static td_api::object_ptr<td_api::LanguagePackString> get_language_pack_string_object(
      const std::pair<string, PluralizedString> &str);
  static td_api::object_ptr<td_api::LanguagePackString> get_language_pack_string_object(const string &str);

  static td_api::object_ptr<td_api::languagePackStrings> get_language_pack_strings_object(Language *language,
                                                                                          const vector<string> &keys);

  void inc_generation();

  static bool is_valid_key(Slice key);

  void save_strings_to_database(Language *language, int32 new_version, vector<std::pair<string, string>> strings);

  void on_get_language_pack_strings(string language_pack, string language_code, int32 version, bool is_diff,
                                    vector<string> keys, vector<tl_object_ptr<telegram_api::LangPackString>> results,
                                    Promise<td_api::object_ptr<td_api::languagePackStrings>> promise);

  void on_failed_get_difference(string language_pack, string language_code);

  void on_result(NetQueryPtr query) override;

  void start_up() override;
  void hangup() override;
  void tear_down() override;

  Container<Promise<NetQueryPtr>> container_;
  void send_with_promise(NetQueryPtr query, Promise<NetQueryPtr> promise);
};

}  // namespace td