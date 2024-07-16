#ifndef ELECTRON_SPIN_SCHEME_URL_LOADER_H_
#define ELECTRON_SPIN_SCHEME_URL_LOADER_H_

#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_context.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

namespace electron_spin {
  using SchemeToLoaderMap = std::map<std::string,mojo::PendingRemote<network::mojom::URLLoaderFactory>>;
  void CreateLoaderFactories(SchemeToLoaderMap* in_out,
                            content::BrowserContext* context,
                            network::mojom::URLLoaderFactory* default_factory,
                            network::mojom::NetworkContext* net_ctxt,
                            PrefService* pref_svc);
}  // namespace electron_spin

#endif  // ELECTRON_SPIN_SCHEME_URL_LOADER_H_
