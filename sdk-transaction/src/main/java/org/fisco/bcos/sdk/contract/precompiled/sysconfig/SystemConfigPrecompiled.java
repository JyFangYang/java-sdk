package org.fisco.bcos.sdk.contract.precompiled.sysconfig;

import java.math.BigInteger;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import org.fisco.bcos.sdk.client.Client;
import org.fisco.bcos.sdk.codec.datatypes.Function;
import org.fisco.bcos.sdk.codec.datatypes.Type;
import org.fisco.bcos.sdk.codec.datatypes.TypeReference;
import org.fisco.bcos.sdk.codec.datatypes.Utf8String;
import org.fisco.bcos.sdk.codec.datatypes.generated.Int256;
import org.fisco.bcos.sdk.codec.datatypes.generated.tuples.generated.Tuple1;
import org.fisco.bcos.sdk.codec.datatypes.generated.tuples.generated.Tuple2;
import org.fisco.bcos.sdk.contract.Contract;
import org.fisco.bcos.sdk.crypto.CryptoSuite;
import org.fisco.bcos.sdk.crypto.keypair.CryptoKeyPair;
import org.fisco.bcos.sdk.model.CryptoType;
import org.fisco.bcos.sdk.model.TransactionReceipt;
import org.fisco.bcos.sdk.model.callback.TransactionCallback;
import org.fisco.bcos.sdk.transaction.model.exception.ContractException;

@SuppressWarnings("unchecked")
public class SystemConfigPrecompiled extends Contract {
    public static final String[] BINARY_ARRAY = {};

    public static final String BINARY =
            org.fisco.bcos.sdk.utils.StringUtils.joinAll("", BINARY_ARRAY);

    public static final String[] SM_BINARY_ARRAY = {};

    public static final String SM_BINARY =
            org.fisco.bcos.sdk.utils.StringUtils.joinAll("", SM_BINARY_ARRAY);

    public static final String[] ABI_ARRAY = {
        "[{\"constant\":false,\"inputs\":[{\"name\":\"key\",\"type\":\"string\"},{\"name\":\"value\",\"type\":\"string\"}],\"name\":\"setValueByKey\",\"outputs\":[{\"name\":\"\",\"type\":\"int256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]"
    };

    public static final String ABI = org.fisco.bcos.sdk.utils.StringUtils.joinAll("", ABI_ARRAY);

    public static final String FUNC_SETVALUEBYKEY = "setValueByKey";

    protected SystemConfigPrecompiled(
            String contractAddress, Client client, CryptoKeyPair credential) {
        super(getBinary(client.getCryptoSuite()), contractAddress, client, credential);
    }

    public static String getBinary(CryptoSuite cryptoSuite) {
        return (cryptoSuite.getCryptoTypeConfig() == CryptoType.ECDSA_TYPE ? BINARY : SM_BINARY);
    }

    public TransactionReceipt setValueByKey(String key, String value) {
        final Function function =
                new Function(
                        FUNC_SETVALUEBYKEY,
                        Arrays.<Type>asList(new Utf8String(key), new Utf8String(value)),
                        Collections.<TypeReference<?>>emptyList());
        return executeTransaction(function);
    }

    public void setValueByKey(String key, String value, TransactionCallback callback) {
        final Function function =
                new Function(
                        FUNC_SETVALUEBYKEY,
                        Arrays.<Type>asList(new Utf8String(key), new Utf8String(value)),
                        Collections.<TypeReference<?>>emptyList());
        asyncExecuteTransaction(function, callback);
    }

    public String getSignedTransactionForSetValueByKey(String key, String value) {
        final Function function =
                new Function(
                        FUNC_SETVALUEBYKEY,
                        Arrays.<Type>asList(new Utf8String(key), new Utf8String(value)),
                        Collections.<TypeReference<?>>emptyList());
        return createSignedTransaction(function);
    }

    public Tuple2<String, String> getSetValueByKeyInput(TransactionReceipt transactionReceipt) {
        String data = transactionReceipt.getInput().substring(10);
        final Function function =
                new Function(
                        FUNC_SETVALUEBYKEY,
                        Arrays.<Type>asList(),
                        Arrays.<TypeReference<?>>asList(
                                new TypeReference<Utf8String>() {},
                                new TypeReference<Utf8String>() {}));
        List<Type> results =
                this.functionReturnDecoder.decode(data, function.getOutputParameters());
        return new Tuple2<String, String>(
                (String) results.get(0).getValue(), (String) results.get(1).getValue());
    }

    public Tuple1<BigInteger> getSetValueByKeyOutput(TransactionReceipt transactionReceipt) {
        String data = transactionReceipt.getOutput();
        final Function function =
                new Function(
                        FUNC_SETVALUEBYKEY,
                        Arrays.<Type>asList(),
                        Arrays.<TypeReference<?>>asList(new TypeReference<Int256>() {}));
        List<Type> results =
                this.functionReturnDecoder.decode(data, function.getOutputParameters());
        return new Tuple1<BigInteger>((BigInteger) results.get(0).getValue());
    }

    public static SystemConfigPrecompiled load(
            String contractAddress, Client client, CryptoKeyPair credential) {
        return new SystemConfigPrecompiled(contractAddress, client, credential);
    }

    public static SystemConfigPrecompiled deploy(Client client, CryptoKeyPair credential)
            throws ContractException {
        return deploy(
                SystemConfigPrecompiled.class,
                client,
                credential,
                getBinary(client.getCryptoSuite()),
                null,
                null,
                null);
    }
}
