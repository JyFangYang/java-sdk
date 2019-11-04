/**
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 *
 * @brief: empty framework for main of FISCO-BCOS
 *
 * @file: dataSyncTool.cpp
 * @author: xingqiangbai
 * @date 2019-10-15
 */
#include "Common.h"
#include "include/BuildInfo.h"
#include "libinitializer/Initializer.h"
#include "libledger/DBInitializer.h"
#include "libledger/LedgerParam.h"
#include "libstorage/BasicRocksDB.h"
#include "libstorage/MemoryTableFactoryFactory2.h"
#include "libstorage/RocksDBStorage.h"
#include "libstorage/RocksDBStorageFactory.h"
#include "libstorage/SQLStorage.h"
#include <boost/algorithm/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <clocale>
#include <ctime>
#include <iostream>
#include <memory>

using namespace std;
using namespace dev;
using namespace boost;
using namespace dev::ledger;
using namespace dev::storage;
using namespace dev::initializer;
namespace fs = boost::filesystem;

uint32_t PageCount = 10000;
uint32_t BigTablePageCount = 50;
const string SYNCED_BLOCK_NUMBER = "#extra_synce_block_number#";

struct SyncRecorder
{
    typedef std::shared_ptr<SyncRecorder> Ptr;

    explicit SyncRecorder(const std::string& path, int64_t _blockNumber) : filename(path + "/.sync")
    {
        if (fs::exists(filename))
        {
            fstream fs(filename);
            boost::archive::text_iarchive ia(fs);
            ia >> tables;
            fs.close();
            m_syncBlock = tables.at(SYNCED_BLOCK_NUMBER).first;
            m_isNewSync = false;
        }
        else
        {
            tables[SYNCED_BLOCK_NUMBER] = make_pair(_blockNumber, false);
            m_syncBlock = _blockNumber;
            m_isNewSync = true;
        }
    }
    ~SyncRecorder() { serialization(); }
    int64_t syncBlock() const { return m_syncBlock; }
    bool isNewSync() const { return m_isNewSync; }
    bool isCompleted(string _tableName) const
    {
        std::lock_guard<std::mutex> l(x_tableStatus);
        return tables.at(_tableName).second;
    }
    uint64_t tableSyncOffset(string _tableName) const
    {
        std::lock_guard<std::mutex> l(x_tableStatus);
        return tables.at(_tableName).first;
    }

    void markStatus(string tableName, pair<uint64_t, bool> status)
    {
        std::lock_guard<std::mutex> l(x_tableStatus);
        tables[tableName] = status;
        serialization();
    }

    void serialization()
    {
        if (!tables.empty())
        {
            ofstream fs(filename, std::fstream::trunc);
            boost::archive::text_oarchive oa(fs);
            oa << tables;
            fs.close();
        }
    }
    mutable std::mutex x_tableStatus;
    unordered_map<string, pair<uint64_t, bool>> tables;
    bool m_isNewSync = true;
    int64_t m_syncBlock;
    string filename;
};

vector<TableInfo::Ptr> parseTableNames(TableData::Ptr data, SyncRecorder::Ptr recorder)
{
    auto entries = data->newEntries;
    vector<TableInfo::Ptr> res;

    for (size_t i = 0; i < entries->size(); ++i)
    {
        auto entry = entries->get(i);
        auto tableInfo = std::make_shared<TableInfo>();
        tableInfo->name = entry->getField("table_name");
        if (tableInfo->name.empty())
        {
            throw std::runtime_error("empty table name");
        }
        if (!recorder->isNewSync() && recorder->isCompleted(tableInfo->name))
        {
            std::cout << tableInfo->name << " already committed." << endl;
            continue;
        }
        else
        {
            tableInfo->key = entry->getField("key_field");
            auto valueFields = entry->getField("value_field");
            boost::split(tableInfo->fields, valueFields, boost::is_any_of(","));
            // new sync insert will faie
            recorder->tables.insert(std::make_pair(tableInfo->name, make_pair(0, false)));
            res.push_back(tableInfo);
        }
    }
    return res;
}

TableData::Ptr getBlockToNonceData(SQLStorage::Ptr _reader, int64_t _blockNumber)
{
    cout << " Process " << SYS_BLOCK_2_NONCES << endl;
    auto tableFactoryFactory = std::make_shared<dev::storage::MemoryTableFactoryFactory2>();
    tableFactoryFactory->setStorage(_reader);
    auto memoryTableFactory = tableFactoryFactory->newTableFactory(dev::h256(), _blockNumber);
    Table::Ptr tb = memoryTableFactory->openTable(SYS_BLOCK_2_NONCES);
    auto entries = tb->select(lexical_cast<std::string>(_blockNumber), tb->newCondition());

    if (entries->size() == 0)
    {
        // ERROR
        return nullptr;
    }
    else
    {
        auto tableInfo = getSysTableInfo(SYS_BLOCK_2_NONCES);
        auto tableData = std::make_shared<TableData>();
        tableData->info = tableInfo;
        tableData->dirtyEntries = std::make_shared<Entries>();
        for (size_t i = 0; i < entries->size(); ++i)
        {
            auto entry = std::make_shared<Entry>();
            entry->copyFrom(entries->get(i));
            tableData->newEntries->addEntry(entry);
        }
        tableData->dirtyEntries = std::make_shared<Entries>();
        return tableData;
    }
}

TableData::Ptr getHashToBlockData(SQLStorage::Ptr _reader, int64_t _blockNumber)
{
    cout << " Process " << SYS_HASH_2_BLOCK << endl;

    auto tableFactoryFactory = std::make_shared<dev::storage::MemoryTableFactoryFactory2>();
    tableFactoryFactory->setStorage(_reader);
    auto memoryTableFactory = tableFactoryFactory->newTableFactory(dev::h256(), _blockNumber);
    Table::Ptr tb = memoryTableFactory->openTable(SYS_NUMBER_2_HASH);
    auto entries = tb->select(lexical_cast<std::string>(_blockNumber), tb->newCondition());
    h256 blockHash;
    if (entries->size() == 0)
    {
        // ERROR
        return nullptr;
    }
    else
    {
        auto entry = entries->get(0);
        blockHash = h256((entry->getField(SYS_VALUE)));
    }
    tb = memoryTableFactory->openTable(SYS_HASH_2_BLOCK);
    entries = tb->select(blockHash.hex(), tb->newCondition());
    if (entries->size() == 0)
    {
        // ERROR
        return nullptr;
    }
    else
    {
        auto tableInfo = getSysTableInfo(SYS_HASH_2_BLOCK);
        auto tableData = std::make_shared<TableData>();
        tableData->info = tableInfo;
        tableData->dirtyEntries = std::make_shared<Entries>();
        for (size_t i = 0; i < entries->size(); ++i)
        {
            auto entry = std::make_shared<Entry>();
            entry->copyFrom(entries->get(i));
            tableData->dirtyEntries->addEntry(entry);
        }
        tableData->newEntries = std::make_shared<Entries>();
        return tableData;
    }
}

void syncData(SQLStorage::Ptr _reader, Storage::Ptr _writer, int64_t _blockNumber,
    const std::string& _dataPath, bool _fullSync)
{
    boost::filesystem::create_directories(_dataPath);
    auto recorder = std::make_shared<SyncRecorder>(_dataPath, _blockNumber);
    auto syncBlock = recorder->syncBlock();
    cout << "sync block number : " << syncBlock << ", data path : " << _dataPath
         << ", new sync : " << recorder->isNewSync() << endl;
    auto sysTableInfo = getSysTableInfo(SYS_TABLES);
    TableData::Ptr sysTableData = std::make_shared<TableData>();
    sysTableData->info = sysTableInfo;
    uint64_t begin = 0;
    while (true)
    {
        auto data = _reader->selectTableDataByNum(syncBlock, sysTableInfo, begin, PageCount);
        for (size_t i = 0; i < data->newEntries->size(); ++i)
        {
            sysTableData->newEntries->addEntry(data->newEntries->get(i));
        }
        if (data->newEntries->size() == 0)
        {
            break;
        }
        auto lastEntry = data->newEntries->get(data->newEntries->size() - 1);
        begin = lastEntry->getID();
        if (data->newEntries->size() < PageCount)
        {
            break;
        }
    }

    auto tableInfos = parseTableNames(sysTableData, recorder);
    if (!recorder->isCompleted(SYS_TABLES))
    {
        _writer->commit(syncBlock, vector<TableData::Ptr>{sysTableData});
        recorder->markStatus(SYS_TABLES, make_pair(begin, true));
        cout << SYS_TABLES << " is committed." << endl;
    }
    auto totalTable = tableInfos.size();
    size_t syncedCount = 1;
    auto pullCommitTableData = [&](TableInfo::Ptr tableInfo, uint64_t start, uint32_t counts) {
        cout << endl
             << "[" << getCurrentDateTime() << "][" << syncedCount << "/" << totalTable
             << "] processing " << tableInfo->name << endl;
        int64_t downloaded = 0;
        while (true)
        {
            auto tableData = _reader->selectTableDataByNum(syncBlock, tableInfo, start, counts);
            if (!tableData)
            {
                cerr << "query failed. Table=" << tableInfo->name << endl;
                break;
            }
            if (tableData->newEntries->size() == 0)
            {
                cout << " |" << tableInfo->name << flush;
                break;
            }
            cout << "\r[" << getCurrentDateTime() << "][" << syncedCount << "/" << totalTable
                 << "] |" << start << flush;
            auto lastEntry = tableData->newEntries->get(tableData->newEntries->size() - 1);
            start = lastEntry->getID();
            _writer->commit(syncBlock, vector<TableData::Ptr>{tableData});
            recorder->markStatus(tableInfo->name, make_pair(start, false));
            downloaded += tableData->newEntries->size();
            cout << " --> " << start << " |downloaded items : " << downloaded << flush;

            if (tableData->newEntries->size() < counts)
            {
                break;
            }
        }
        recorder->markStatus(tableInfo->name, make_pair(start, true));
        ++syncedCount;
        cout << " done.\r" << flush;
    };

    // SYS_HASH_2_BLOCK
    if (!recorder->isCompleted(SYS_HASH_2_BLOCK))
    {
        if (!_fullSync)
        {
            auto data = getHashToBlockData(_reader, syncBlock);
            _writer->commit(syncBlock, vector<TableData::Ptr>{data});
            recorder->markStatus(SYS_HASH_2_BLOCK, make_pair(data->newEntries->size(), true));
        }
        else
        {
            auto tableInfo = getSysTableInfo(SYS_HASH_2_BLOCK);
            pullCommitTableData(
                tableInfo, recorder->tableSyncOffset(tableInfo->name), BigTablePageCount);
        }
    }
    // SYS_BLOCK_2_NONCES
    if (!recorder->isCompleted(SYS_BLOCK_2_NONCES))
    {
        if (!_fullSync)
        {
            auto data = getBlockToNonceData(_reader, syncBlock);
            if (data)
            {
                _writer->commit(syncBlock, vector<TableData::Ptr>{data});
                recorder->markStatus(SYS_BLOCK_2_NONCES, make_pair(data->newEntries->size(), true));
            }
            else
            {
                cout << "block number to nonce is empty at " << syncBlock << endl;
            }
        }
        else
        {
            auto tableInfo = getSysTableInfo(SYS_BLOCK_2_NONCES);
            pullCommitTableData(
                tableInfo, recorder->tableSyncOffset(tableInfo->name), BigTablePageCount);
        }
    }

    for (const auto& tableInfo : tableInfos)
    {
        if (tableInfo->name == SYS_TABLES || tableInfo->name == SYS_BLOCK_2_NONCES ||
            tableInfo->name == SYS_HASH_2_BLOCK)
        {
            continue;
        }
        pullCommitTableData(tableInfo, recorder->tableSyncOffset(tableInfo->name), PageCount);
    }
}

void fastSyncGroupData(std::shared_ptr<LedgerParamInterface> _param,
    ChannelRPCServer::Ptr _channelRPCServer, int64_t _rollbackNumber = 1000)
{
    // create SQLStorage
    auto sqlStorage = createSQLStorage(_param, _channelRPCServer, [](std::exception& e) {
        LOG(ERROR) << LOG_BADGE("STORAGE") << LOG_BADGE("MySQL")
                   << "access mysql failed exit:" << e.what();
        raise(SIGTERM);
        BOOST_THROW_EXCEPTION(e);
    });
    auto p = dynamic_pointer_cast<SQLStorage>(sqlStorage);
    p->setTimeout(15);
    auto blockNumber = getBlockNumberFromStorage(sqlStorage);
    blockNumber = blockNumber >= _rollbackNumber ? blockNumber - _rollbackNumber : 0;

    // create writer
    Storage::Ptr writerStorage;
    bool fullSync = true;
    if (!dev::stringCmpIgnoreCase(_param->mutableStorageParam().type, "External"))
    {
        cout << "error unsupported external storage" << endl;
        exit(0);
    }
    else if (!dev::stringCmpIgnoreCase(_param->mutableStorageParam().type, "MySQL"))
    {
        writerStorage = createZdbStorage(_param, [](std::exception& e) {
            LOG(ERROR) << LOG_BADGE("STORAGE") << LOG_BADGE("MySQL")
                       << "access mysql failed exit:" << e.what();
            raise(SIGTERM);
            BOOST_THROW_EXCEPTION(e);
        });
    }
    else if (!dev::stringCmpIgnoreCase(_param->mutableStorageParam().type, "RocksDB"))
    {
        writerStorage =
            createRocksDBStorage(_param->mutableStorageParam().path, false, false, true);
    }
    else
    {
        fullSync = false;
        auto scalableStorage =
            std::make_shared<ScalableStorage>(_param->mutableStorageParam().scrollThreshold);
        auto rocksDBStorageFactory = std::make_shared<RocksDBStorageFactory>(
            _param->mutableStorageParam().path + "/blocksDB",
            _param->mutableStorageParam().binaryLog, false);
        rocksDBStorageFactory->setDBOpitons(getRocksDBOptions());
        scalableStorage->setStorageFactory(rocksDBStorageFactory);
        // make RocksDBStorage think cachedStorage is exist
        auto stateStorage =
            createRocksDBStorage(_param->mutableStorageParam().path + "/state", false, false, true);
        scalableStorage->setStateStorage(stateStorage);
        auto archiveStorage = rocksDBStorageFactory->getStorage(to_string(blockNumber));
        scalableStorage->setArchiveStorage(archiveStorage, blockNumber);
        scalableStorage->setRemoteBlockNumber(blockNumber);
        writerStorage = scalableStorage;
    }

    // fast sync data
    syncData(dynamic_pointer_cast<SQLStorage>(sqlStorage), writerStorage, blockNumber,
        _param->mutableStorageParam().path, fullSync);
}

int main(int argc, const char* argv[])
{
    /// set LC_ALL
    setDefaultOrCLocale();
    std::set_terminate([]() {
        std::cerr << "terminate handler called" << endl;
        abort();
    });
    std::cout << "fisco-sync version : "
              << "0.1.0" << std::endl;
    std::cout << "Build Time         : " << FISCO_BCOS_BUILD_TIME << std::endl;
    std::cout << "Commit Hash        : " << FISCO_BCOS_COMMIT_HASH << std::endl;
    /// init params
    std::cout << "[" << getCurrentDateTime() << "] "
              << "The sync-tool is Initializing..." << std::endl;
    boost::program_options::options_description main_options("Usage of fisco-sync");
    main_options.add_options()("help,h", "print help information")("config,c",
        boost::program_options::value<std::string>()->default_value("./config.ini"),
        "config file path, eg. config.ini")("verify,v",
        boost::program_options::value<int64_t>()->default_value(1000),
        "verify number of blocks")("limit,l",
        boost::program_options::value<uint32_t>()->default_value(10000), "page counts of table")(
        "sys_limit,s", boost::program_options::value<uint32_t>()->default_value(50),
        "page counts of system table")(
        "group,g", boost::program_options::value<uint>()->default_value(1), "sync specific group");
    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, main_options), vm);
    }
    catch (...)
    {
        std::cout << "invalid parameters" << std::endl;
        std::cout << main_options << std::endl;
        exit(0);
    }
    if (vm.count("help") || vm.count("h"))
    {
        std::cout << main_options << std::endl;
        exit(0);
    }
    int64_t verifyBlocks = vm["verify"].as<int64_t>();
    verifyBlocks = verifyBlocks < 100 ? 100 : verifyBlocks;
    PageCount = vm["limit"].as<uint32_t>();
    BigTablePageCount = vm["sys_limit"].as<uint32_t>();
    string configPath = vm["config"].as<std::string>();
    int groupID = vm["group"].as<uint>();

    try
    {
        /// init log
        boost::property_tree::ptree pt;
        boost::property_tree::read_ini(configPath, pt);
        auto logInitializer = std::make_shared<LogInitializer>();
        logInitializer->initLog(pt);

        /// init global config. must init before DB, for compatibility
        initGlobalConfig(pt);
        /// init channelServer
        auto secureInitializer = std::make_shared<SecureInitializer>();
        secureInitializer->initConfig(pt);

        auto rpcInitializer = std::make_shared<RPCInitializer>();
        rpcInitializer->setSSLContext(
            secureInitializer->SSLContext(SecureInitializer::Usage::ForRPC));
        auto p2pService = std::make_shared<Service>();
        rpcInitializer->setP2PService(p2pService);
        rpcInitializer->initChannelRPCServer(pt);
        auto groupConfigPath = pt.get<string>("group.group_config_path", "conf/");
        auto dataPath = pt.get<string>("group.group_data_path", "data/");
        boost::filesystem::path path(groupConfigPath);
        if (fs::is_directory(path))
        {
            fs::directory_iterator endIter;
            for (fs::directory_iterator iter(path); iter != endIter; iter++)
            {
                if (fs::extension(*iter) == ".genesis" &&
                    iter->path().stem().string() == "group." + to_string(groupID))
                {
                    std::cout << "[" << getCurrentDateTime() << "] The sync-tool is syncing group "
                              << groupID << ". config file " << iter->path().string() << std::endl;
                    auto params = std::make_shared<LedgerParam>();
                    params->init(iter->path().string(), dataPath);
                    fastSyncGroupData(params, rpcInitializer->channelRPCServer(), verifyBlocks);
                    std::cout << endl
                              << "[" << getCurrentDateTime() << "] sync complete." << std::endl;
                    return 0;
                }
            }
            std::cout << "[" << getCurrentDateTime() << "] "
                      << "Can't find genesis and ini config of group" << groupID << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << boost::diagnostic_information(e);
        std::cerr << "sync failed!!!" << std::endl;
        return -1;
    }

    return 0;
}
